#include <errno.h>
#include <arpa/inet.h>
#include "xmn_socket.h"
#include "xmn_macro.h"
#include "xmn_func.h"
#include "xmn_memory.h"
#include "xmn_comm.h"

void XMNSocket::WaitRequestHandler(XMNConnSockInfo *pconnsockinfo)
{
    /**
     * 从接收缓冲区中取数据。
    */
    ssize_t recvcount = RecvData(pconnsockinfo, pconnsockinfo->pdataheaderstart, pconnsockinfo->recvlen);
    if (recvcount <= 0)
    {
        return;
    }
    /****************************************************
     * 
     * 程序走到了这里，说明确实收到了数据。
     * 下面是一组状态机的实现，用于保证接收到的数据是完整的。
     * 
    ****************************************************/
    /**
     * （1）开始接收包头数据。
    */

    xmn_log_stderr(0, "Data is arrived.");

    if (pconnsockinfo->recvstat == PKG_HD_INIT)
    {
        /**
         * 完整地接收到了包头数据。
        */
        if (recvcount == pkgheaderlen_)
        {
            WaitRequestHandlerHeader(pconnsockinfo);
        }
        /**
         * 接收的数据不足一个包头的长度，需要为下次接收数据做准备。
        */
        else
        {
            pconnsockinfo->recvstat = PKG_HD_RECVING;
            pconnsockinfo->pdataheaderstart = pconnsockinfo->dataheader + recvcount;
            pconnsockinfo->recvlen = pkgheaderlen_ - recvcount;
        }
    }
    /**
     * （2）上回包头数据没有接收完整，现在继续接收包头数据。
    */
    else if (pconnsockinfo->recvstat == PKG_HD_RECVING)
    {
        /**
         * 包头接收完毕。
        */
        if (pconnsockinfo->recvlen == recvcount)
        {
            WaitRequestHandlerHeader(pconnsockinfo);
        }
        else
        {
            pconnsockinfo->recvstat = PKG_HD_RECVING;
            pconnsockinfo->pdataheaderstart = pconnsockinfo->pdataheaderstart + recvcount;
            pconnsockinfo->recvlen = pkgheaderlen_ - recvcount;
        }
    }
    /**
     * （3）包头数据接收完整了，现在开始接收包体数据。
    */
    else if (pconnsockinfo->recvstat == PKG_BD_INIT)
    {
        if (pconnsockinfo->recvlen == recvcount)
        {
            WaitRequestHandlerBody(pconnsockinfo);
        }
        else
        {
            pconnsockinfo->recvstat = PKG_BD_RECVING;
            pconnsockinfo->pdataheaderstart = pconnsockinfo->pdataheaderstart + recvcount;
            pconnsockinfo->recvlen = pkgheaderlen_ - recvcount;
        }
    }
    /**
     * （4）上回包体数据没有接收完整，现在继续接收包体数据。
    */
    else if (pconnsockinfo->recvstat == PKG_BD_RECVING)
    {
        if (pconnsockinfo->recvlen == recvcount)
        {
            WaitRequestHandlerBody(pconnsockinfo);
        }
        else
        {
            pconnsockinfo->recvstat = PKG_BD_RECVING;
            pconnsockinfo->pdataheaderstart = pconnsockinfo->pdataheaderstart + recvcount;
            pconnsockinfo->recvlen = pkgheaderlen_ - recvcount;
        }
    }

    return;
}

ssize_t XMNSocket::RecvData(XMNConnSockInfo *pconnsockinfo, char *pbuff, const size_t &bufflen)
{
    ssize_t n = 0;
    /**
     * （1）接收数据。
    */
    n = recv(pconnsockinfo->fd, pbuff, bufflen, 0);
    /**
     * （2）对 recv 的返回值进行判断处理。
    */
    if (n == 0)
    {
        /**
         * 客户端已正常关闭，即：完成了 4 次挥手。
        */
        CloseConnection(pconnsockinfo);
        return 0;
    }
    else if (n < 0)
    {
        /**
         * recv 没有数据了，一般在 ET 模式下出现该 errno，
         * 用来标识接收缓冲区中已经没有数据了。
         * 如果是 LT 模式，不应该出现该 errno，因为 LT 模式下，
         * 没有数据时 epoll_wait 是不会返回的。
        */
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            xmn_log_stderr(errno, "XMNSocket::RecvData() 返回了 EAGAIN 或者 EWOULDBLOCK 错误。");
            return 0;
        }
        /**
         * TODO：这里需要补充该 errno 的值的意义。
        */
        else if (errno == EINTR)
        {
            xmn_log_stderr(errno, "XMNSocket::RecvData() 返回了 EINTR 错误。");
            return 0;
        }
        /**
         * 下面的错误都是异常，需要关闭连接 socket 并回收连接至连接池。
        */
        if (errno == ECONNRESET)
        {
            /**
             * client 向 server 发送了 RST 包，发送该包的原因是 client 正在通信，
             * 而 client 突然被关闭，导致了与 server 没有进行正常的 4 次挥手，
             * 而是发送了 RST 包。
            */
            xmn_log_stderr(errno, "XMNSocket::RecvData() 返回了 ECONNRESET 错误，即：client -> server rst 包。");
        }
        else
        {
            xmn_log_stderr(errno, "XMNSocket::RecvData() 返回了未知错误。");
        }
        CloseConnection(pconnsockinfo);
        return -1;
    }

    return n;
}

void XMNSocket::WaitRequestHandlerHeader(XMNConnSockInfo *pconnsockinfo)
{
    /**
     * （1）判断该包是否正常,若不正常，则直接将状态机复原为初始状态。
    */
    unsigned short pkglen = 0;
    XMNPkgHeader *ppkgheader = (XMNPkgHeader *)pconnsockinfo->dataheader;
    pkglen = ntohs(ppkgheader->pkglen);
    if (pkglen < pkgheaderlen_)
    {
        pconnsockinfo->recvstat = PKG_HD_INIT;
        pconnsockinfo->pdataheaderstart = pconnsockinfo->dataheader;
        pconnsockinfo->recvlen = pkgheaderlen_;
    }
    else if (pkglen > PKG_MAX_LEN)
    {
        pconnsockinfo->recvstat = PKG_HD_INIT;
        pconnsockinfo->pdataheaderstart = pconnsockinfo->dataheader;
        pconnsockinfo->recvlen = pkgheaderlen_;
    }
    else
    {
        /**
        * （2）为包体分配内存并设置相关变量。
        */
        XMNMemory *pmemory = XMNMemory::GetInstance();
        char *pbuffall = (char *)pmemory->AllocMemory(msgheaderlen_ + pkglen, false);
        if (pbuffall == nullptr)
        {
            /**
             * TODO：申请内存失败的情况怎么处理暂时没有想好，先返回。
            */
            return;
        }
        pconnsockinfo->precvalldata = pbuffall;
        pconnsockinfo->isfree = true;
        /**
         * a、处理消息头。
        */
        XMNMsgHeader *pmsgheader = (XMNMsgHeader *)pbuffall;
        pmsgheader->pconnsockinfo = pconnsockinfo;
        pmsgheader->currsequence = pconnsockinfo->currsequence;
        /**
         * b、处理包头。
        */
        pbuffall += msgheaderlen_;
        XMNPkgHeader *ppkgheadertmp = (XMNPkgHeader *)pbuffall;
        memcpy(ppkgheadertmp, ppkgheader, sizeof(XMNPkgHeader) * 1);
        /**
         * 处理 client 向 server 仅仅发送包头的情况。
        */
        if (pkglen == pkgheaderlen_)
        {
            WaitRequestHandlerBody(pconnsockinfo);
        }
        /**
         * c、更新状态机。
        */
        else
        {
            pconnsockinfo->recvstat = PKG_BD_INIT;
            pconnsockinfo->pdataheaderstart = pbuffall + pkgheaderlen_;
            pconnsockinfo->recvlen = pkglen - pkgheaderlen_;
        }
    }

    return;
}

void XMNSocket::WaitRequestHandlerBody(XMNConnSockInfo *pconnsockinfo)
{
    /**
     * （1）将接收的数据压入消息队列中。
    */
    /**
     * TODO：返回值为-1暂时没有想好怎么处理。
    */
    PutInRecvMsgList(pconnsockinfo->precvalldata);
    /**
     * （2）更新状态机至初始状态。
    */
    pconnsockinfo->isfree = false;
    pconnsockinfo->recvstat = PKG_HD_INIT;
    pconnsockinfo->pdataheaderstart = pconnsockinfo->dataheader;
    pconnsockinfo->recvlen = pkgheaderlen_;
    pconnsockinfo->precvalldata = nullptr;
    return;
}

int XMNSocket::PutInRecvMsgList(char *pdata)
{
    if (pdata == nullptr)
    {
        return -1;
    }
    xmn_log_stderr(errno, "完整地收到了一包。");
    return 0;
}