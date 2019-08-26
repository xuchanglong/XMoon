#include "xmn_func.h"
#include "xmn_socket.h"
#include <string.h>

XMNConnSockInfo *XMNSocket::GetConnSockInfo(const int &fd)
{
    /**
     * （1）从连接池中取出未用的连接。
    */
    XMNConnSockInfo *pconnsockinfo = pfree_connsock_list_head_;
    if (pconnsockinfo == nullptr)
    {
        xmn_log_stderr(errno, "GetConnSockInfo 空闲链表为空！");
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
    //后续补充……

    /**
     * 将保留的信息重新赋值。
    */
    pconnsockinfo->instance = instance;
    /**
     * 每次取节点时该值都加1。
    */
    pconnsockinfo->currsequence = currsequence;
    ++pconnsockinfo->currsequence;
    return pconnsockinfo;
}