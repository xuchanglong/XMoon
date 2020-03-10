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

void XMNSocket::WaitReadRequestHandler(std::shared_ptr<XMNConnSockInfo>  connsockinfo)
{
    /**
     * （1）从接收缓冲区中取数据。
    */
    ssize_t recvcount = RecvData(connsockinfo);
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
    //XMNLogStdErr(0, "Data is arrived.");
    if (connsockinfo->recvstatus == PKG_HD_INIT)
    {
        /**
         * 完整地接收到了包头数据。
        */
        if (recvcount == kPkgHeaderLen_)
        {
            WaitRequestHandlerHeader(connsockinfo);
        }
        /**
         * 接收的数据不足一个包头的长度，需要为下次接收数据做准备。
        */
        else
        {
            connsockinfo->recvstatus = PKG_HD_RECVING;
            connsockinfo->precvdatastart += recvcount;
            connsockinfo->recvdatalen -= recvcount;
        }
    }

    /**
     * （2）上回包头数据没有接收完整，现在继续接收包头数据。
    */
    else if (connsockinfo->recvstatus == PKG_HD_RECVING)
    {
        /**
         * 包头接收完毕。
        */
        if (connsockinfo->recvdatalen == recvcount)
        {
            WaitRequestHandlerHeader(connsockinfo);
        }
        else
        {
            connsockinfo->recvstatus = PKG_HD_RECVING;
            connsockinfo->precvdatastart += recvcount;
            connsockinfo->recvdatalen -= recvcount;
        }
    }

    /**
     * （3）包头数据接收完整了，现在开始接收包体数据。
    */
    else if (connsockinfo->recvstatus == PKG_BD_INIT)
    {
        if (connsockinfo->recvdatalen == recvcount)
        {
            WaitRequestHandlerBody(connsockinfo);
        }
        else
        {
            connsockinfo->recvstatus = PKG_BD_RECVING;
            connsockinfo->precvdatastart += recvcount;
            connsockinfo->recvdatalen -= recvcount;
        }
    }

    /**
     * （4）上回包体数据没有接收完整，现在继续接收包体数据。
    */
    else if (connsockinfo->recvstatus == PKG_BD_RECVING)
    {
        if (connsockinfo->recvdatalen == recvcount)
        {
            WaitRequestHandlerBody(connsockinfo);
        }
        else
        {
            connsockinfo->recvstatus = PKG_BD_RECVING;
            connsockinfo->precvdatastart += recvcount;
            connsockinfo->recvdatalen -= recvcount;
        }
    }

    return;
}

ssize_t XMNSocket::RecvData(std::shared_ptr<XMNConnSockInfo>  connsockinfo)
{
    ssize_t n = 0;
    char *pbuff = connsockinfo->precvdatastart;
    const size_t kBuffLen = connsockinfo->recvdatalen;
    /**
     * （1）接收数据。
    */
    n = recv(connsockinfo->fd, pbuff, kBuffLen, 0);

    /**
     * （2）对 recv 的返回值进行判断处理。
    */
    if (n == 0)
    {
        /**
         * 客户端已正常关闭，即：完成了 4 次挥手。
        */
        if (close(connsockinfo->fd) == -1)
        {
            XMNLogStdErr(0, "XMNSocket::RecvData 中 close 执行失败。");
        }
        //CloseConnection(pconnsockinfo);
        //XMNLogStdErr(0,"connsockinfo put in recylist.");
        /**
         * 延时回收连接。
        */
        PutInConnSockInfo2RecyList(connsockinfo);
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
            XMNLogStdErr(errno, "XMNSocket::RecvData() 返回了 EAGAIN 或者 EWOULDBLOCK 错误。");
            return 0;
        }
        /**
         * TODO：这里需要补充该 errno 的值的意义。
        */
        else if (errno == EINTR)
        {
            XMNLogStdErr(errno, "XMNSocket::RecvData() 返回了 EINTR 错误。");
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
            XMNLogStdErr(errno, "XMNSocket::RecvData() 返回了 ECONNRESET 错误，即：client -> server rst 包。");
        }
        else
        {
            XMNLogStdErr(errno, "XMNSocket::RecvData() 返回了未知错误。");
        }

        if (close(connsockinfo->fd) == -1)
        {
            XMNLogStdErr(0, "XMNSocket::RecvData 中 close 执行失败。");
        }
        //CloseConnection(pconnsockinfo);
        XMNLogStdErr(0, "connsockinfo put in recylist.");
        /**
         * 延时回收连接。
        */
        PutInConnSockInfo2RecyList(connsockinfo);
        return -1;
    }

    return n;
}

void XMNSocket::WaitRequestHandlerHeader(std::shared_ptr<XMNConnSockInfo>  connsockinfo)
{
    /**
     * （1）判断该包是否正常,若不正常，则直接将状态机复原为初始状态。
    */
    unsigned short pkglen = 0;
    XMNPkgHeader *ppkgheader = (XMNPkgHeader *)connsockinfo->precvdatastart;
    pkglen = ntohs(ppkgheader->pkglen);
    if ((pkglen < kPkgHeaderLen_) || (pkglen > PKG_MAX_LEN))
    {
        connsockinfo->recvstatus = PKG_HD_INIT;
        connsockinfo->precvdatastart = connsockinfo->dataheader;
        connsockinfo->recvdatalen = kPkgHeaderLen_;
    }
    else
    {
        /**
        * （2）为包体分配内存并设置相关变量。
        */
        XMNMemory &memory = SingletonBase<XMNMemory>::GetInstance();
        char *pbuffall = (char *)memory.AllocMemory(kMsgHeaderLen_ + pkglen, false);
        if (pbuffall == nullptr)
        {
            /**
             * TODO：申请内存失败的情况怎么处理暂时没有想好，先返回。
            */
            return;
        }
        connsockinfo->precvalldata = pbuffall;
        connsockinfo->isfree = true;

        /**
         * a、处理消息头。
        */
        XMNMsgHeader *pmsgheader = (XMNMsgHeader *)pbuffall;
        pmsgheader->connsockinfo = connsockinfo;
        pmsgheader->currsequence = connsockinfo->currsequence;

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
            WaitRequestHandlerBody(connsockinfo);
        }

        /**
         * c、更新状态机。
        */
        else
        {
            connsockinfo->recvstatus = PKG_BD_INIT;
            connsockinfo->precvdatastart = pbuffall + kPkgHeaderLen_;
            connsockinfo->recvdatalen = pkglen - kPkgHeaderLen_;
        }
    }

    return;
}

void XMNSocket::WaitRequestHandlerBody(std::shared_ptr<XMNConnSockInfo>  connsockinfo)
{
    bool isflood = false;
    if (floodattackmonitorenable_)
    {
        isflood = TestFlood(connsockinfo);
        if (isflood)
        {
            XMNLogStdErr(0, "3");
            XMNMemory &memory = SingletonBase<XMNMemory>::GetInstance();
            memory.FreeMemory(connsockinfo->precvalldata);
        }
        else
        {
            /**
             * （1）将接收的数据压入消息队列中。
            */
            /**
             * TODO：返回值为-1暂时没有想好怎么处理。
            */
            g_threadpool.PutInRecvMsgList_Signal(connsockinfo->precvalldata);
        }
    }

    /**
     * （2）更新状态机至初始状态。
    */
    connsockinfo->isfree = false;
    connsockinfo->recvstatus = PKG_HD_INIT;
    connsockinfo->precvdatastart = connsockinfo->dataheader;
    connsockinfo->recvdatalen = kPkgHeaderLen_;
    connsockinfo->precvalldata = nullptr;

    /**
     * （3）若是 flood 攻击，则断开连接。
    */
    if (isflood)
    {
        //XMNLogStdErr(0, "该连接存在恶意攻击，已断开连接。");
        ActivelyCloseSocket(connsockinfo);
    }

    return;
}

void XMNSocket::ThreadRecvProcFunc(char *pmsgbuf)
{
    ;
}

void XMNSocket::WaitWriteRequestHandler(std::shared_ptr<XMNConnSockInfo>  connsockinfo)
{
    XMNMemory &memory = SingletonBase<XMNMemory>::GetInstance();
    /**
     * （1）发送消息。
    */
    ssize_t sendsize = SendData(connsockinfo);

    /**
     * （2）对发送数据的结果进行处理。
    */
    if (sendsize > 0 && sendsize != connsockinfo->senddatalen)
    {
        /**
         * 发送成功，但是没有发全，下次继续发送。
        */
        connsockinfo->psenddata = connsockinfo->psenddata + sendsize;
        connsockinfo->senddatalen = connsockinfo->senddatalen - sendsize;
        return;
    }
    else if (sendsize == -1)
    {
        /**
         * 发送缓冲区已满不太可能，因为 epoll 驱动通知系统可以发送，
         * 结果发送缓冲区已满，不正常。
        */
        XMNLogStdErr(0, "XMNSocket::WaitWriteRequestHandler()执行 SendData 时发现发送缓冲区已满的问题。");
        return;
    }
    else if (sendsize > 0 && sendsize == connsockinfo->senddatalen)
    {
        /**
         * 发送成功且发全。
         * 在 epoll 红黑树中删掉该 socket 。
        */
        if (EpollOperationEvent(connsockinfo->fd,
                                EPOLL_CTL_MOD,
                                EPOLLOUT,
                                1,
                                connsockinfo) != 0)
        {
            XMNLogStdErr(0, "XMNSocket::WaitWriteRequestHandler()中EpollOperationEvent()执行失败。");
        }
        XMNLogStdErr(0, "XMNSocket::WaitWriteRequestHandler()发送数据完毕。");
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
        XMNLogStdErr(0, "XMNSocket::WaitWriteRequestHandler()执行失败。");
    }
    memory.FreeMemory(connsockinfo->psendalldataforfree);
    connsockinfo->psendalldataforfree = nullptr;
    connsockinfo->psenddata = nullptr;
    connsockinfo->senddatalen = 0;
    --connsockinfo->throwepollsendcount;
}