#include "comm/xmn_socket.h"
#include "xmn_func.h"
#include "xmn_memory.h"

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
    ++currsequence;
    recvstat = PKG_HD_INIT;
    precvdatastart = dataheader;
    recvdatalen = sizeof(XMNPkgHeader);
    precvalldata = nullptr;
    psendalldata = nullptr;
    eventtype = 0;
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
    /**
     * （1）从连接池中取出未用的连接。
    */
    XMNConnSockInfo *pconnsockinfo = pfree_connsock_list_head_;
    if (pconnsockinfo == nullptr)
    {
        xmn_log_stderr(errno, "PutOutConnSockInfofromPool 空闲链表为空！");
        return nullptr;
    }
    /**
     * 更新指向空闲链表的首节点。
    */
    pfree_connsock_list_head_ = pconnsockinfo->next;
    /**
     * 连接池中未用的节点数量 - 1 。
    */
    --pool_free_connsock_count_;

    /**
     * （2）对取出的节点进行清空和赋值操作。
    */
    /**
     * 保存有用的信息。
    */
    uint64_t currsequence = pconnsockinfo->currsequence;
    unsigned int instance = pconnsockinfo->instance;
    /**
     * 赋值。
    */
    memset(pconnsockinfo, 0, sizeof(struct XMNConnSockInfo) * 1);
    pconnsockinfo->fd = fd;
    pconnsockinfo->recvstat = PKG_HD_INIT;
    pconnsockinfo->precvdatastart = pconnsockinfo->dataheader;
    pconnsockinfo->recvdatalen = pkgheaderlen_;

    /**
     * 将保留的信息重新赋值。
     * 这些保留信息的值都是为了判定 epoll_wait 返回的 epoll_event 是否为过期事件。
    */
    pconnsockinfo->instance = !instance;
    pconnsockinfo->currsequence = currsequence;
    /**
     * 每次取节点时该值都加1。
    */
    ++pconnsockinfo->currsequence;
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