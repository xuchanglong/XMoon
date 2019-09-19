#include "comm/xmn_socket.h"
#include "xmn_func.h"
#include "xmn_memory.h"
#include "xmn_lockmutex.hpp"
#include <pthread.h>

#include <string.h>

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
    XMNLockMutex connsockpoolmutex(&connsockpoolmutex);
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
    /**
     * （1）释放存放完整数据包的内存。
    */
    if (pconnsockinfo->isfree)
    {
        SingletonBase<XMNMemory>::GetInstance()->FreeMemory((void *)pconnsockinfo->precvalldata);
        pconnsockinfo->isfree = false;
        pconnsockinfo->precvalldata = nullptr;
    }

    /**
     * （2）将连接插入空闲连接链表中。
    */
    pconnsockinfo->next = pfree_connsock_list_head_;

    ++pconnsockinfo->currsequence;

    pfree_connsock_list_head_ = pconnsockinfo;
    ++pool_free_connsock_count_;
    return;
}