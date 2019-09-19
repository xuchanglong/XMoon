#include "comm/xmn_socket.h"
#include "xmn_func.h"
#include "xmn_memory.h"
#include "xmn_lockmutex.hpp"
#include <pthread.h>

#include <string.h>

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
     * 序号 + 1，用于判断消息是否过期。
    */
    ++currsequence;

    /**
     * 收包状态机变为初始化状态。
    */
    recvstat = PKG_HD_INIT;
    precvdatastart = dataheader;
    recvdatalen = sizeof(XMNPkgHeader);

    /**
     * 其他变量初始化。
    */
    precvalldata = nullptr;
    psendalldata = nullptr;
    eventtype = 0;
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

    if (psendalldata != nullptr)
    {
        pmemory->FreeMemory((void *)psendalldata);
        psendalldata = nullptr;
    }
    
}

/**************************************************************************************
 * 
 ***************** XMNSocket 相关函数 **************** 
 * 
**************************************************************************************/
void XMNSocket::InitConnSockInfoPool()
{
    XMNMemory *pmemory = SingletonBase<XMNMemory>::GetInstance();
    size_t connsockinfolen = sizeof(XMNConnSockInfo);
    for (size_t i = 0; i < worker_connection_count_; i++)
    {
        XMNConnSockInfo *pconnsockinfo = (XMNConnSockInfo *)pmemory->AllocMemory(connsockinfolen, false);
        pconnsockinfo = new (pconnsockinfo) XMNConnSockInfo();
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

XMNConnSockInfo *XMNSocket::PutOutConnSockInfofromPool(const int &fd)
{
    XMNLockMutex connsockpoolmutex(&connsock_pool_mutex_);
    XMNConnSockInfo *pconnsockinfo = nullptr;
    if (pool_free_connsock_count_)
    {
        /**
         * 控线连接池中有连接。
        */
        pconnsockinfo = connsock_pool_free_.front();
        connsock_pool_free_.pop_front();
        --pool_free_connsock_count_;
    }
    else
    {
        /**
         * 空闲连接池中无连接，在创建一个连接。
        */
        XMNMemory *pmemory = SingletonBase<XMNMemory>::GetInstance();
        pconnsockinfo = (XMNConnSockInfo *)pmemory->AllocMemory(sizeof(XMNConnSockInfo));
        pconnsockinfo = new(pconnsockinfo)XMNConnSockInfo();
        connsock_pool_.push_back(pconnsockinfo);
        ++pool_connsock_count_;
    }
    pconnsockinfo->InitConnSockInfo();
    pconnsockinfo->fd = fd;
    
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
        xmn_log_info(XMN_LOG_ALERT, errno, "CloseConnection 中 close (%d) 失败！", fd);
    }

    return;
}