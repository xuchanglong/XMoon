#include "sys/socket.h"
#include "sys/types.h"
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "xmn_socket.h"
#include "xmn_macro.h"
#include "xmn_func.h"

void XMNSocket::EventAcceptHandler(XMNConnSockInfo *pconnsockinfo)
{
    /**
     * （1）变量申请。
    */
    struct sockaddr addr;
    socklen_t addrlen = sizeof(struct sockaddr);
    memset(&addr, 0, sizeof(struct sockaddr) * 1);
    static bool isuseaccept4 = true;
    int linkfd = -1;
    int listenfd = pconnsockinfo->plistensockinfo->fd;
    XMNConnSockInfo *pconnsockinfo_new = nullptr;
    /**
     * 用于临时存储 errno 的值，防止在对不同的错误进行处理时，其他代码修改了该值。
    */
    int errnotmp = 0;
    int logerrorlevel = 0;

    /**
     * （2）接收 client 发来的请求连接的数据。
    */
    do
    {
        /**
         * 创建 listenfd 时，设置为非堵塞，故该函数会立刻返回。
        */
        if (isuseaccept4)
        {
            linkfd = accept4(listenfd, &addr, &addrlen, SOCK_NONBLOCK);
        }
        else
        {
            linkfd = accept(listenfd, &addr, &addrlen);
        }

        /**
         * （3）对 accept4 或者  accept 的返回值进行判断处理。
        *       非0     有 client 连入。
        *       -1      有错误发生，通过 errno 获取报错代码。
        */
        if (linkfd == -1)
        {
            errnotmp = errno;
            /**
             *  对于 accept、send 和 recv 而言，事件未发生时， errno 通常被设置为 EAGAIN 
             *  或者 EWOULDBLOCK（期待堵塞），说明 accept 没有准备好，直接返回即可。 
            */
            if (errnotmp == EAGAIN)
            {
                return;
            }
            logerrorlevel = XMN_LOG_ALERT;
            /**
             * 该错误被描述为“software caused connection abort”，即“软件引起的连接中止”。
             * 原因在于当服务和客户进程在完成用于 TCP 连接的“三次握手”后，客户 TCP 却发送了一个 RST （复位）分节。
             * 在服务进程看来，就在该连接已由 TCP 排队，等着服务进程调用 accept 的时候 RST 却到达了。
             * POSIX 规定此时的 errno 值必须 ECONNABORTED。
             * 源自 Berkeley 的实现完全在内核中处理中止的连接，服务进程将永远不知道该中止的发生。
             * 服务器进程一般可以忽略该错误，直接再次调用accept。
            */
            if (errnotmp == ECONNABORTED)
            {
                logerrorlevel = XMN_LOG_ERR;
            }
            xmn_log_info(logerrorlevel, errnotmp, "EventAcceptHandler 中 accept4 或者 accept 执行失败！");
            /**
             * accept4() 系统没有实现。
            */
            if (isuseaccept4 && errnotmp == ENOSYS)
            {
                isuseaccept4 = false;
                continue;
            }

            if (errnotmp == ECONNABORTED)
            {
                /**
                 * 该错误可以忽略，不做处理。
                */
            }

            if ((errnotmp == EMFILE) || (errnotmp == ENFILE))
            {
                /**
                 * 这个官方做法是先把读事件从listen socket上移除，
                 * 然后再弄个定时器，定时器到了则继续执行该函数，
                 * 但是定时器到了有个标记，会把读事件增加到listen socket上去；
                */
                /**
                 * TODO：具体操作后续补充。
                */
            }
            return;
        }
        /**
         * 执行到这里，说明 accept 或者 accept4 执行成功了。 
        */
        pconnsockinfo_new = GetConnSockInfo(linkfd);
        if (pconnsockinfo_new == nullptr)
        {
            /**
             *  连接池中的连接不够用，那么直接将 linkfd 关闭即可。
            */
            if (close(linkfd) == -1)
            {
                xmn_log_info(XMN_LOG_ALERT, errno, "EventAcceptHandler 中 close (%d) 失败！", linkfd);
            }
            return;
        }
        /**
         * 执行到这里说明已经成功地从连接池中拿到了一个连接。
        */
        memcpy(&pconnsockinfo_new->sockaddrinfo, &addr, sizeof(struct sockaddr) * 1);

        if (!isuseaccept4)
        {
            if (SetNonBlocking(linkfd) != 0)
            {
                /**
                 * 回收连接至连接池并关闭 socket 。
                */
                CloseConnection(pconnsockinfo_new);
                return;
            }
        }
        /**
         * 连接对象和监听对象关联。
        */
        pconnsockinfo_new->plistensockinfo = pconnsockinfo->plistensockinfo;
        /**
         * 标记该连接是可写的。
        */
        pconnsockinfo_new->w_ready = 1;
        /**
         *  设置数据来时读处理函数。
        */
        pconnsockinfo_new->rhandler = &XMNSocket::WaitRequestHandler;
        /**
        * （4）将新建立的连接加入到 epoll 的红黑树中。
        */
        int r = EpollAddEvent(linkfd, 1, 0, EPOLLET, EPOLL_CTL_ADD, pconnsockinfo_new);
        if (r != 0)
        {
            CloseConnection(pconnsockinfo_new);
            return -4;
        }
        break;
    } while (true);

    return 0;
}

void XMNSocket::CloseConnection(XMNConnSockInfo *pfd)
{
    /**
     * 先回收连接的目的是防止 close 失败导致连接无法回收。
    */
    int fd = pfd->fd;
    FreeConnSockInfo(pfd);
    pfd->fd = -1;
    if (close(fd) == -1)
    {
        xmn_log_info(XMN_LOG_ALERT, errno, "CloseConnection 中 close (%d) 失败！", fd);
    }

    return;
}