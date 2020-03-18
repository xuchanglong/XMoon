#include "comm/xmn_socket.h"
#include "xmn_func.h"
#include "xmn_memory.h"
#include "xmn_lockmutex.hpp"
#include "xmn_macro.h"
#include "xmn_global.h"
#include "xmn_mempool.hpp"

#include <pthread.h>
#include <string.h>
#include <unistd.h>

/**************************************************************************************
 * 
 ***************** XMNConnSockInfo 相关函数 **************** 
 * 
**************************************************************************************/
XMNConnSockInfo::XMNConnSockInfo()
{
    memset(this, 0, sizeof(struct XMNConnSockInfo));
    int r = pthread_mutex_init(&logicprocmutex_, nullptr);
    if (r != 0)
    {
        XMNLogStdErr(0, "XMNConnSockInfo::XMNConnSockInfo() 调用 pthread_mutex_init 失败，错误代码：%d", r);
    }
}

XMNConnSockInfo::~XMNConnSockInfo()
{
    int r = pthread_mutex_destroy(&logicprocmutex_);
    if (r != 0)
    {
        XMNLogStdErr(0, "XMNConnSockInfo::~XMNConnSockInfo() 调用 pthread_mutex_destroy 失败，错误代码：%d", r);
    }
}

void XMNConnSockInfo::InitConnSockInfo()
{
    /**
     * （1）序号 + 1，用于判断消息是否过期。
    */
    ++currsequence;

    /**
     * （2）收包状态机变为初始化状态。
    */
    recvstatus = PKG_HD_INIT;
    precvdatastart = dataheader;
    recvdatalen = sizeof(XMNPkgHeader);

    /**
     * （3）其他变量初始化。
    */
    precvalldata = nullptr;
    psendalldataforfree = nullptr;
    psenddata = nullptr;
    events = 0;
    throwepollsendcount = 0;
}

void XMNConnSockInfo::ClearConnSockInfo()
{
    /**
     * 序号 + 1，用于判断消息是否过期。
    */
    ++currsequence;

    /**
     * 释放内存。
    */
    XMNMemory &memory = SingletonBase<XMNMemory>::GetInstance();
    if (precvalldata != nullptr)
    {
        memory.FreeMemory((void *)precvalldata);
        precvalldata = nullptr;
    }

    if (psendalldataforfree != nullptr)
    {
        //memory.FreeMemory((void *)psendalldataforfree);
        // TODO：对不同的处理函数要有不同的处理。
        SingletonBase<XMNMemPool<RegisterInfoAll>>::GetInstance().DeAllocate(psendalldataforfree);
        psendalldataforfree = nullptr;
    }

    /**
     * 其他变量清零。
    */
    throwepollsendcount = 0;
}

/**************************************************************************************
 * 
 ***************** XMNSocket 相关函数 **************** 
 * 
**************************************************************************************/

XMNConnSockInfo *XMNSocket::PutOutConnSockInfofromPool(const int &kSockFd)
{
    XMNLockMutex connsockpoolmutex(&connsock_pool_mutex_);

    XMNConnSockInfo *pconnsockinfo = (XMNConnSockInfo *)SingletonBase<XMNMemPool<XMNConnSockInfo>>::GetInstance().Allocate();
    pconnsockinfo = new (pconnsockinfo) XMNConnSockInfo();

    pconnsockinfo->InitConnSockInfo();
    pconnsockinfo->fd = kSockFd;

    return pconnsockinfo;
}

void XMNSocket::PutInConnSockInfo2Pool(XMNConnSockInfo *pconnsockinfo)
{
    XMNLockMutex connsockinfomutex(&connsock_pool_mutex_);
    pconnsockinfo->ClearConnSockInfo();
    SingletonBase<XMNMemPool<XMNConnSockInfo>>::GetInstance().DeAllocate(pconnsockinfo);
    return;
}

void XMNSocket::PutInConnSockInfo2RecyList(XMNConnSockInfo *pconnsockinfo)
{
    XMNLockMutex connsockinfomutex(&connsock_pool_recycle_mutex_);

    /**
     * 防止连接被重复地放入连接回收池中。
    */
    for (const auto &x : recycleconnsock_pool_)
    {
        if (x == pconnsockinfo)
        {
            return;
        }
    }

    pconnsockinfo->putinrecylisttime = time(nullptr);
    ++pconnsockinfo->currsequence;
    recycleconnsock_pool_.push_back(pconnsockinfo);
    ++pool_recyconnsock_count_;

    --onlineuser_count_;
    return;
}

void *XMNSocket::ConnSockInfoRecycleThread(void *pthreadinfo)
{
    if (pthreadinfo == nullptr)
    {
        return nullptr;
    }
    ThreadInfo *pthreadinfo_n = (ThreadInfo *)pthreadinfo;
    XMNSocket *pthis = pthreadinfo_n->pthis_;
    XMNConnSockInfo *pconnsockinfo = nullptr;
    while (true)
    {
        usleep(200 * 1000);
        if (pthis->pool_recyconnsock_count_)
        {
            time_t curtime = time(nullptr);
            pthread_mutex_lock(&pthis->connsock_pool_recycle_mutex_);
            std::list<XMNConnSockInfo *>::iterator it;
            for (it = pthis->recycleconnsock_pool_.begin(); it != pthis->recycleconnsock_pool_.end();)
            {
                pconnsockinfo = *it;
                if (!g_isquit && (curtime - pconnsockinfo->putinrecylisttime) < pthis->recyconnsockinfowaittime_)
                {
                    /**
                     * 没有到时间 ，继续等待。
                    */
                    ++it;
                    continue;
                }
                /**
                 * 有到达等待时间的连接则回收。
                */
                if (pconnsockinfo->throwepollsendcount != 0)
                {
                    /**
                     * 这种情况不应该发生，记录一下，以备寻找错误。
                    */
                    XMNLogStdErr(0, "XMNSocket::ConnSockInfoRecycleThread() 连接回收了throwepollsendcount!=0，不应该发生。");
                }

                it = pthis->recycleconnsock_pool_.erase(it);
                --pthis->pool_recyconnsock_count_;
                pthis->PutInConnSockInfo2Pool(pconnsockinfo);

                //XMNLogStdErr(0,"connsockinfo is recycled.");
            }
            pthread_mutex_unlock(&pthis->connsock_pool_recycle_mutex_);
        }
        if (g_isquit)
        {
            if (pthis->pool_recyconnsock_count_)
            {
                pthread_mutex_lock(&pthis->connsock_pool_recycle_mutex_);
                std::list<XMNConnSockInfo *>::iterator it;
                for (it = pthis->recycleconnsock_pool_.begin(); it != pthis->recycleconnsock_pool_.end();)
                {
                    pconnsockinfo = *it;
                    it = pthis->recycleconnsock_pool_.erase(it);
                    --pthis->pool_recyconnsock_count_;
                    pthis->PutInConnSockInfo2Pool(pconnsockinfo);
                }
                pthread_mutex_unlock(&pthis->connsock_pool_recycle_mutex_);
            }
            break;
        }
    }

    return nullptr;
}

void XMNSocket::CloseConnection(XMNConnSockInfo *pconnsockinfo)
{
    /**
     * 先回收连接的目的是防止 close 失败导致连接无法回收。
    */
    PutInConnSockInfo2Pool(pconnsockinfo);
    if (close(pconnsockinfo->fd) == -1)
    {
        XMNLogInfo(XMN_LOG_ALERT, errno, "CloseConnection 中 close (%d) 失败！", pconnsockinfo->fd);
    }

    return;
}

size_t XMNSocket::ConnectPoolSize()
{
    return SingletonBase<XMNMemPool<XMNConnSockInfo>>::GetInstance().MemBlockCount();
}