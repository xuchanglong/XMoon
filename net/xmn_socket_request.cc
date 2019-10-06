#include "comm/xmn_socket.h"
#include "xmn_macro.h"
#include "xmn_func.h"
#include "xmn_memory.h"
#include "comm/xmn_socket_comm.h"
#include "xmn_lockmutex.hpp"
#include "xmn_global.h"

#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>

void XMNSocket::WaitReadRequestHandler(XMNConnSockInfo *pconnsockinfo)
{
    /**
     * （1）从接收缓冲区中取数据。
    */
    ssize_t recvcount = RecvData(pconnsockinfo);
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
     * （2）开始接收包头数据。
    */
    //xmn_log_stderr(0, "Data is arrived.");
    if (pconnsockinfo->recvstatus == PKG_HD_INIT)
    {
        /**
         * 完整地接收到了包头数据。
        */
        if (recvcount == kPkgHeaderLen_)
        {
            WaitRequestHandlerHeader(pconnsockinfo);
        }
        /**
         * 接收的数据不足一个包头的长度，需要为下次接收数据做准备。
        */
        else
        {
            pconnsockinfo->recvstatus = PKG_HD_RECVING;
            pconnsockinfo->precvdatastart += recvcount;
            pconnsockinfo->recvdatalen -= recvcount;
        }
    }

    /**
     * （2）上回包头数据没有接收完整，现在继续接收包头数据。
    */
    else if (pconnsockinfo->recvstatus == PKG_HD_RECVING)
    {
        /**
         * 包头接收完毕。
        */
        if (pconnsockinfo->recvdatalen == recvcount)
        {
            WaitRequestHandlerHeader(pconnsockinfo);
        }
        else
        {
            pconnsockinfo->recvstatus = PKG_HD_RECVING;
            pconnsockinfo->precvdatastart += recvcount;
            pconnsockinfo->recvdatalen -= recvcount;
        }
    }

    /**
     * （3）包头数据接收完整了，现在开始接收包体数据。
    */
    else if (pconnsockinfo->recvstatus == PKG_BD_INIT)
    {
        if (pconnsockinfo->recvdatalen == recvcount)
        {
            WaitRequestHandlerBody(pconnsockinfo);
        }
        else
        {
            pconnsockinfo->recvstatus = PKG_BD_RECVING;
            pconnsockinfo->precvdatastart += recvcount;
            pconnsockinfo->recvdatalen -= recvcount;
        }
    }

    /**
     * （4）上回包体数据没有接收完整，现在继续接收包体数据。
    */
    else if (pconnsockinfo->recvstatus == PKG_BD_RECVING)
    {
        if (pconnsockinfo->recvdatalen == recvcount)
        {
            WaitRequestHandlerBody(pconnsockinfo);
        }
        else
        {
            pconnsockinfo->recvstatus = PKG_BD_RECVING;
            pconnsockinfo->precvdatastart += recvcount;
            pconnsockinfo->recvdatalen -= recvcount;
        }
    }

    return;
}

ssize_t XMNSocket::RecvData(XMNConnSockInfo *pconnsockinfo)
{
    ssize_t n = 0;
    char *pbuff = pconnsockinfo->precvdatastart;
    const size_t kBuffLen = pconnsockinfo->recvdatalen;
    /**
     * （1）接收数据。
    */
    n = recv(pconnsockinfo->fd, pbuff, kBuffLen, 0);

    /**
     * （2）对 recv 的返回值进行判断处理。
    */
    if (n == 0)
    {
        /**
         * 客户端已正常关闭，即：完成了 4 次挥手。
        */
        if (close(pconnsockinfo->fd) == -1)
        {
            xmn_log_stderr(0, "XMNSocket::RecvData 中 close 执行失败。");
        }
        //CloseConnection(pconnsockinfo);
        //xmn_log_stderr(0,"connsockinfo put in recylist.");
        /**
         * 延时回收连接。
        */
        PutInConnSockInfo2RecyList(pconnsockinfo);
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

        if (close(pconnsockinfo->fd) == -1)
        {
            xmn_log_stderr(0, "XMNSocket::RecvData 中 close 执行失败。");
        }
        //CloseConnection(pconnsockinfo);
        xmn_log_stderr(0, "connsockinfo put in recylist.");
        /**
         * 延时回收连接。
        */
        PutInConnSockInfo2RecyList(pconnsockinfo);
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
    XMNPkgHeader *ppkgheader = (XMNPkgHeader *)pconnsockinfo->precvdatastart;
    pkglen = ntohs(ppkgheader->pkglen);
    if ((pkglen < kPkgHeaderLen_) || (pkglen > PKG_MAX_LEN))
    {
        pconnsockinfo->recvstatus = PKG_HD_INIT;
        pconnsockinfo->precvdatastart = pconnsockinfo->dataheader;
        pconnsockinfo->recvdatalen = kPkgHeaderLen_;
    }
    else
    {
        /**
        * （2）为包体分配内存并设置相关变量。
        */
        XMNMemory *pmemory = SingletonBase<XMNMemory>::GetInstance();
        char *pbuffall = (char *)pmemory->AllocMemory(kMsgHeaderLen_ + pkglen, false);
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
        pbuffall += kMsgHeaderLen_;
        XMNPkgHeader *ppkgheadertmp = (XMNPkgHeader *)pbuffall;
        memcpy(ppkgheadertmp, ppkgheader, sizeof(XMNPkgHeader) * 1);

        /**
         * 处理 client 向 server 仅仅发送包头的情况。
        */
        if (pkglen == kPkgHeaderLen_)
        {
            WaitRequestHandlerBody(pconnsockinfo);
        }

        /**
         * c、更新状态机。
        */
        else
        {
            pconnsockinfo->recvstatus = PKG_BD_INIT;
            pconnsockinfo->precvdatastart = pbuffall + kPkgHeaderLen_;
            pconnsockinfo->recvdatalen = pkglen - kPkgHeaderLen_;
        }
    }

    return;
}

void XMNSocket::WaitRequestHandlerBody(XMNConnSockInfo *pconnsockinfo)
{
    bool isflood = false;
    if (floodattackmonitorenable_)
    {
        isflood = TestFlood(pconnsockinfo);
        if (isflood)
        {
            xmn_log_stderr(0, "3");
            XMNMemory *pmemory = SingletonBase<XMNMemory>::GetInstance();
            pmemory->FreeMemory(pconnsockinfo->precvalldata);
        }
        else
        {
            /**
             * （1）将接收的数据压入消息队列中。
            */
            /**
             * TODO：返回值为-1暂时没有想好怎么处理。
            */
            g_threadpool.PutInRecvMsgList_Signal(pconnsockinfo->precvalldata);
        }
    }

    /**
     * （2）更新状态机至初始状态。
    */
    pconnsockinfo->isfree = false;
    pconnsockinfo->recvstatus = PKG_HD_INIT;
    pconnsockinfo->precvdatastart = pconnsockinfo->dataheader;
    pconnsockinfo->recvdatalen = kPkgHeaderLen_;
    pconnsockinfo->precvalldata = nullptr;

    /**
     * （3）若是 flood 攻击，则断开连接。
    */
    if (isflood)
    {
        //xmn_log_stderr(0, "该连接存在恶意攻击，已断开连接。");
        ActivelyCloseSocket(pconnsockinfo);
    }

    return;
}

void XMNSocket::ThreadRecvProcFunc(char *pmsgbuf)
{
    ;
}

void XMNSocket::WaitWriteRequestHandler(XMNConnSockInfo *pconnsockinfo)
{
    XMNMemory *pmemory = SingletonBase<XMNMemory>::GetInstance();
    /**
     * （1）发送消息。
    */
    ssize_t sendsize = SendData(pconnsockinfo);

    /**
     * （2）对发送数据的结果进行处理。
    */
    if (sendsize > 0 && sendsize != pconnsockinfo->senddatalen)
    {
        /**
         * 发送成功，但是没有发全，下次继续发送。
        */
        pconnsockinfo->psenddata = pconnsockinfo->psenddata + sendsize;
        pconnsockinfo->senddatalen = pconnsockinfo->senddatalen - sendsize;
        return;
    }
    else if (sendsize == -1)
    {
        /**
         * 发送缓冲区已满不太可能，因为 epoll 驱动通知系统可以发送，
         * 结果发送缓冲区已满，不正常。
        */
        xmn_log_stderr(0, "XMNSocket::WaitWriteRequestHandler()执行 SendData 时发现发送缓冲区已满的问题。");
        return;
    }
    else if (sendsize > 0 && sendsize == pconnsockinfo->senddatalen)
    {
        /**
         * 发送成功，包已发全。
         * 在 epoll 红黑树中删掉该 socket 。
        */
        if (EpollOperationEvent(pconnsockinfo->fd,
                                EPOLL_CTL_MOD,
                                EPOLLOUT,
                                1,
                                pconnsockinfo) != 0)
        {
            xmn_log_stderr(0, "XMNSocket::WaitWriteRequestHandler()中EpollOperationEvent()执行失败。");
        }
        xmn_log_stderr(0, "XMNSocket::WaitWriteRequestHandler()发送数据完毕。");
    }

    /**
     * （3）收尾工作。
     * a、数据完整地发送完毕。
     * b、对端断开连接。
     * c、未知错误。
    */
    /**
     * 增加信号量。
    */
    if (sem_post(&senddata_queue_sem_) != 0)
    {
        xmn_log_stderr(0, "XMNSocket::WaitWriteRequestHandler()执行失败。");
    }
    pmemory->FreeMemory(pconnsockinfo->psendalldataforfree);
    pconnsockinfo->psendalldataforfree = nullptr;
    pconnsockinfo->psenddata = nullptr;
    pconnsockinfo->senddatalen = 0;
    --pconnsockinfo->throwepollsendcount;
}