#include <errno.h>
#include "xmn_socket.h"
#include "xmn_macro.h"
#include "xmn_func.h"

void XMNSocket::WaitRequestHandler(XMNConnSockInfo *pconnsockinfo)
{
    ssize_t recvcount = RecvData(pconnsockinfo, pconnsockinfo->pdataheaderstart, pconnsockinfo->recvlen);
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