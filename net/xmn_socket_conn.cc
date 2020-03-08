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
        XMNLogStdErr(0, "XMNConnSockInfo::XMNConnSockInfo() 调用 pthread_mutex_init 失败，错误代码：%d", r);
    }
}

XMNConnSockInfo::~XMNConnSockInfo()
{
    int r = pthread_mutex_destroy(&logicprocmutex);
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
        memory.FreeMemory((void *)psendalldataforfree);
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
    XMNMemory &memory = SingletonBase<XMNMemory>::GetInstance();
    const size_t kConnSockInfoLen = sizeof(XMNConnSockInfo);

    /**
     * 为连接池创建 worker_connection_count_ 个连接，后续可以再增加。
    */
    for (size_t i = 0; i < worker_connection_count_; ++i)
    {
        XMNConnSockInfo *pconnsockinfo = (XMNConnSockInfo *)memory.AllocMemory(kConnSockInfoLen, false);
        std::shared_ptr<XMNConnSockInfo> connsockinfo(new (pconnsockinfo) XMNConnSockInfo());
        connsockinfo->InitConnSockInfo();
        connsock_pool_.push_back(connsockinfo);
        connsock_pool_free_.push_back(connsockinfo);
    }
    pool_connsock_count_ = connsock_pool_.size();
    pool_free_connsock_count_ = connsock_pool_free_.size();
}

void XMNSocket::FreeConnSockInfoPool()
{
    std::shared_ptr<XMNConnSockInfo> connsockinfo;
    XMNMemory &memory = SingletonBase<XMNMemory>::GetInstance();
    while (!connsock_pool_.empty())
    {
        //connsockinfo = connsock_pool_.front();
        connsock_pool_.pop_front();
        //pconnsockinfo->~XMNConnSockInfo();
        //memory.FreeMemory((void *)pconnsockinfo);
    }
}

std::shared_ptr<XMNConnSockInfo> XMNSocket::PutOutConnSockInfofromPool(const int &kSockFd)
{
    XMNLockMutex connsockpoolmutex(&connsock_pool_mutex_);
    std::shared_ptr<XMNConnSockInfo> connsockinfo;
    if (pool_free_connsock_count_)
    {
        /**
         * 空闲连接池中有连接。
        */
        connsockinfo = connsock_pool_free_.front();
        connsock_pool_free_.pop_front();
        --pool_free_connsock_count_;
    }
    else
    {
        /**
         * 空闲连接池中无连接，再创建一个连接。
        */
        XMNMemory &memory = SingletonBase<XMNMemory>::GetInstance();
        XMNConnSockInfo *pconnsockinfo = (XMNConnSockInfo *)memory.AllocMemory(sizeof(XMNConnSockInfo), false);
        connsockinfo = std::shared_ptr<XMNConnSockInfo>(new (pconnsockinfo) XMNConnSockInfo());
        connsock_pool_.push_back(connsockinfo);
        ++pool_connsock_count_;
    }
    connsockinfo->InitConnSockInfo();
    connsockinfo->fd = kSockFd;

    return connsockinfo;
}

void XMNSocket::PutInConnSockInfo2Pool(std::shared_ptr<XMNConnSockInfo> &connsockinfo)
{
    XMNLockMutex connsockinfomutex(&connsock_pool_mutex_);
    connsockinfo->ClearConnSockInfo();
    connsock_pool_free_.push_back(connsockinfo);
    ++pool_free_connsock_count_;
    return;
}

void XMNSocket::PutInConnSockInfo2RecyList(std::shared_ptr<XMNConnSockInfo> &connsockinfo)
{
    XMNLockMutex connsockinfomutex(&connsock_pool_recycle_mutex_);

    /**
     * 防止连接被重复地放入连接回收池中。
    */
    for (const auto &x : recycleconnsock_pool_)
    {
        if (x == connsockinfo)
        {
            return;
        }
    }

    connsockinfo->putinrecylisttime = time(nullptr);
    ++connsockinfo->currsequence;
    recycleconnsock_pool_.push_back(connsockinfo);
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
    std::shared_ptr<ThreadInfo> threadinfo_n = *(std::shared_ptr<ThreadInfo> *)pthreadinfo;
    XMNSocket *pthis = threadinfo_n->pthis_;
    std::shared_ptr<XMNConnSockInfo> connsockinfo;
    while (true)
    {
        usleep(200 * 1000);
        if (pthis->pool_recyconnsock_count_)
        {
            time_t curtime = time(nullptr);
            pthread_mutex_lock(&pthis->connsock_pool_recycle_mutex_);
            std::list<std::shared_ptr<XMNConnSockInfo>>::iterator it;
            for (it = pthis->recycleconnsock_pool_.begin(); it != pthis->recycleconnsock_pool_.end();)
            {
                connsockinfo = *it;
                if (!g_isquit && (curtime - connsockinfo->putinrecylisttime) < pthis->recyconnsockinfowaittime_)
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
                if (connsockinfo->throwepollsendcount != 0)
                {
                    /**
                     * 这种情况不应该发生，记录一下，以备寻找错误。
                    */
                    XMNLogStdErr(0, "XMNSocket::ConnSockInfoRecycleThread() 连接回收了throwepollsendcount!=0，不应该发生。");
                }

                it = pthis->recycleconnsock_pool_.erase(it);
                --pthis->pool_recyconnsock_count_;
                pthis->PutInConnSockInfo2Pool(connsockinfo);

                //XMNLogStdErr(0,"connsockinfo is recycled.");
            }
            pthread_mutex_unlock(&pthis->connsock_pool_recycle_mutex_);
        }
        if (g_isquit)
        {
            if (pthis->pool_recyconnsock_count_)
            {
                pthread_mutex_lock(&pthis->connsock_pool_recycle_mutex_);
                std::list<std::shared_ptr<XMNConnSockInfo>>::iterator it;
                for (it = pthis->recycleconnsock_pool_.begin(); it != pthis->recycleconnsock_pool_.end();)
                {
                    connsockinfo = *it;
                    it = pthis->recycleconnsock_pool_.erase(it);
                    --pthis->pool_recyconnsock_count_;
                    pthis->PutInConnSockInfo2Pool(connsockinfo);
                }
                pthread_mutex_unlock(&pthis->connsock_pool_recycle_mutex_);
            }
            break;
        }
    }

    return nullptr;
}

void XMNSocket::CloseConnection(std::shared_ptr<XMNConnSockInfo> &connsockinfo)
{
    /**
     * 先回收连接的目的是防止 close 失败导致连接无法回收。
    */
    PutInConnSockInfo2Pool(connsockinfo);
    if (close(connsockinfo->fd) == -1)
    {
        XMNLogInfo(XMN_LOG_ALERT, errno, "CloseConnection 中 close (%d) 失败！", connsockinfo->fd);
    }

    return;
}