#include "comm/xmn_socket.h"
#include "xmn_func.h"
#include "xmn_memory.h"
#include "xmn_lockmutex.hpp"
#include "xmn_macro.h"
#include "xmn_global.h"

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
    int r = pthread_mutex_init(&logicprocmutex, nullptr);
    if (r != 0)
    {
        xmn_log_stderr(0, "XMNConnSockInfo::XMNConnSockInfo() 调用 pthread_mutex_init 失败，错误代码：%d", r);
    }
}

XMNConnSockInfo::~XMNConnSockInfo()
{
    int r = pthread_mutex_destroy(&logicprocmutex);
    if (r != 0)
    {
        xmn_log_stderr(0, "XMNConnSockInfo::~XMNConnSockInfo() 调用 pthread_mutex_destroy 失败，错误代码：%d", r);
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
    XMNMemory *pmemory = SingletonBase<XMNMemory>::GetInstance();
    if (precvalldata != nullptr)
    {
        pmemory->FreeMemory((void *)precvalldata);
        precvalldata = nullptr;
    }

    if (psendalldataforfree != nullptr)
    {
        pmemory->FreeMemory((void *)psendalldataforfree);
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
void XMNSocket::InitConnSockInfoPool()
{
    XMNMemory *pmemory = SingletonBase<XMNMemory>::GetInstance();
    const size_t kConnSockInfoLen = sizeof(XMNConnSockInfo);

    /**
     * 为连接池创建 worker_connection_count_ 个连接，后续可以再增加。
    */
    for (size_t i = 0; i < worker_connection_count_; ++i)
    {
        XMNConnSockInfo *pconnsockinfo = (XMNConnSockInfo *)pmemory->AllocMemory(kConnSockInfoLen, false);
        pconnsockinfo = new (pconnsockinfo) XMNConnSockInfo();
        pconnsockinfo->InitConnSockInfo();
        connsock_pool_.push_back(pconnsockinfo);
        connsock_pool_free_.push_back(pconnsockinfo);
    }
    pool_connsock_count_ = connsock_pool_.size();
    pool_free_connsock_count_ = connsock_pool_free_.size();
}

void XMNSocket::FreeConnSockInfoPool()
{
    XMNConnSockInfo *pconnsockinfo = nullptr;
    XMNMemory *pmemory = SingletonBase<XMNMemory>::GetInstance();
    while (!connsock_pool_.empty())
    {
        pconnsockinfo = connsock_pool_.front();
        connsock_pool_.pop_front();
        pconnsockinfo->~XMNConnSockInfo();
        pmemory->FreeMemory((void *)pconnsockinfo);
    }
}

XMNConnSockInfo *XMNSocket::PutOutConnSockInfofromPool(const int &kSockFd)
{
    XMNLockMutex connsockpoolmutex(&connsock_pool_mutex_);
    XMNConnSockInfo *pconnsockinfo = nullptr;
    if (pool_free_connsock_count_)
    {
        /**
         * 空闲连接池中有连接。
        */
        pconnsockinfo = connsock_pool_free_.front();
        connsock_pool_free_.pop_front();
        --pool_free_connsock_count_;
    }
    else
    {
        /**
         * 空闲连接池中无连接，再创建一个连接。
        */
        XMNMemory *pmemory = SingletonBase<XMNMemory>::GetInstance();
        pconnsockinfo = (XMNConnSockInfo *)pmemory->AllocMemory(sizeof(XMNConnSockInfo), false);
        pconnsockinfo = new (pconnsockinfo) XMNConnSockInfo();
        connsock_pool_.push_back(pconnsockinfo);
        ++pool_connsock_count_;
    }
    pconnsockinfo->InitConnSockInfo();
    pconnsockinfo->fd = kSockFd;

    return pconnsockinfo;
}

void XMNSocket::PutInConnSockInfo2Pool(XMNConnSockInfo *pconnsockinfo)
{
    XMNLockMutex connsockinfomutex(&connsock_pool_mutex_);
    pconnsockinfo->ClearConnSockInfo();
    connsock_pool_free_.push_back(pconnsockinfo);
    ++pool_free_connsock_count_;
    return;
}

void XMNSocket::PutInConnSockInfo2RecyList(XMNConnSockInfo *pconnsockinfo)
{
    XMNLockMutex connsockinfomutex(&connsock_pool_recy_mutex_);

    pconnsockinfo->putinrecylisttime = time(nullptr);
    ++pconnsockinfo->currsequence;
    recyconnsock_pool_.push_back(pconnsockinfo);
    ++pool_recyconnsock_count_;
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
            pthread_mutex_lock(&pthis->connsock_pool_recy_mutex_);
            std::list<XMNConnSockInfo *>::iterator it;
            for (it = pthis->recyconnsock_pool_.begin(); it != pthis->recyconnsock_pool_.end();)
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
                    xmn_log_stderr(0, "XMNSocket::ConnSockInfoRecycleThread() 连接回收了throwepollsendcount!=0，不应该发生。");
                }

                it = pthis->recyconnsock_pool_.erase(it);
                --pthis->pool_recyconnsock_count_;
                pthis->PutInConnSockInfo2Pool(pconnsockinfo);

                //xmn_log_stderr(0,"connsockinfo is recycled.");
            }
            pthread_mutex_unlock(&pthis->connsock_pool_recy_mutex_);
        }
        if (g_isquit)
        {
            if (pthis->pool_recyconnsock_count_)
            {
                pthread_mutex_lock(&pthis->connsock_pool_recy_mutex_);
                std::list<XMNConnSockInfo *>::iterator it;
                for (it = pthis->recyconnsock_pool_.begin(); it != pthis->recyconnsock_pool_.end();)
                {
                    pconnsockinfo = *it;
                    it = pthis->recyconnsock_pool_.erase(it);
                    --pthis->pool_recyconnsock_count_;
                    pthis->PutInConnSockInfo2Pool(pconnsockinfo);
                }
                pthread_mutex_unlock(&pthis->connsock_pool_recy_mutex_);
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
        xmn_log_info(XMN_LOG_ALERT, errno, "CloseConnection 中 close (%d) 失败！", pconnsockinfo->fd);
    }

    return;
}