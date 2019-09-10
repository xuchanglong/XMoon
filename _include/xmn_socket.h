/*****************************************************************************************
 * 
 *  @function linux socket 配置文件。
 *  @author xuchanglong
 *  @time   2019-08-25
 * 
 *****************************************************************************************/
#ifndef XMOON__INCLUDE_XMN_SOCKET_H_
#define XMOON__INCLUDE_XMN_SOCKET_H_

#include <vector>
#include <cstddef>
#include <sys/epoll.h>
#include <sys/socket.h>

#include "base/noncopyable.h"
#include "xmn_comm.h"

using CXMNSocket = class XMNSocket;
using xmn_event_handler = void (CXMNSocket::*)(struct XMNConnSockInfo *pconnsockinfo);

/**
 * 存放已经完成连接的 socket 的队列的大小。
*/
#define XMN_LISTEN_BACKLOG 511

/**
 * 存放每次从 epoll_wait 的双向链表中取出的 epoll_event 的最大数量。
*/
#define XMN_EPOLL_WAIT_MAX_EVENTS 512

/**
 *  @function   存放监听 socket 的相关的信息。
 *  @author xuchanglong
 *  @time   2019-08-25
*/
struct XMNListenSockInfo
{
    /**
     * 监听端口号。
    */
    size_t port;

    /**
     * 监听 socket 。
    */
    int fd;

    /**
     * 该监听 socket 对应的连接池中的连接。
    */
    XMNConnSockInfo *pconnsockinfo;
};

/**
 * @function    存放连接 socket 的相关信息。
 * @author  xuchanglong
 * @time    2019-08-26
*/
struct XMNConnSockInfo
{
    /**
     * 指向下一个该类型的对象。
    */
    XMNConnSockInfo *next;

    /**
     * 连接 socket 。
    */
    int fd;

    /**
     * 该连接 socket 对应的监听 socket 的信息。
    */
    XMNListenSockInfo *plistensockinfo;

    /**
     * 位域，失效的标志位。
     * 0    有效
     * 1    失效
    */
    unsigned int instance : 1;

    /**
     * 序号。
    */
    uint64_t currsequence;

    /**
     * 保存 client 地址信息用的。
    */
    struct sockaddr sockaddrinfo;

    /**
     * 读准备好标志。
    */
    uint8_t r_ready;

    /**
     * 写准备好标志。
    */
    uint8_t w_ready;

    /**
     * 读事件相关处理函数。
    */
    xmn_event_handler rhandler;

    /**
     * 写事件相关处理函数。
    */
    xmn_event_handler whandler;

    /****************************************************
     * 与收包相关的变量。
    ****************************************************/
    /**
     * 接收当前数据包的状态。
    */
    RecvStatus recvstat;

    /**
     * 存放包头数据。
    */
    char dataheader[XMN_PKG_HEADER_SIZE];

    /**
     * 当前需要接收的数据应该存放在 dataheader 的中的位置。
    */
    char *pdataheaderstart;

    /**
     * 当前需要接收的数据的字节数。
    */
    size_t recvlen;

    /**
     * 存放最终接收的 消息头 + 包头 + 包体。
    */
    char *precvalldata;

    /**
     * 标记 precvalldata 是否需要释放。
    */
    bool isfree;
};

/****************************************************
 * 
 * 消息头，在收到的每一个消息的前面添加消息头。
 * 用于记录该消息对应的连接以及连接序号。
 * 
****************************************************/
struct XMNMsgHeader
{
    /**
     * 指定该消息记录对应的是哪个连接。
    */
    XMNConnSockInfo *pconnsockinfo;

    /**
     * 指定该消息对应的连接的序号，用于判断该连接是否是过期连接。
     * 因为同一个 XMNConnSockInfo 可能对应出多个连接。
    */
    uint64_t currsequence;
};

class XMNSocket : public NonCopyable
{
public:
    XMNSocket();
    virtual ~XMNSocket();

public:
    /**
     *  @function   按照配置文件创建指定数量的监听 socket 。
     *  @paras  none 。
     *  @return 0   操作成功。
     *                  1   配置文件中端口号数量 <= 0 。
     *                  -1  sokcet 创建失败。
     *                  -2  setsockopt 设置失败。
     *                  -3  SetNonBlocking 设置失败。
     *                  -4  bind 绑定失败。
     *                  -5  listen  监听失败。
     *  @time   2019-08-25
    */
    virtual int Initialize();

public:
    /**
     * @function    初始化 epoll 功能。
     * @paras   none 。
     * @return  0   操作成功。
     * @time    2019-08-26
    */
    int EpollInit();

    /**
     * @function    向 epoll 中增加事件。
     * @paras   fd  被 epoll 监控的 socket 。
     *                  readevent   表示添加的事件是否是读事件，1是，0反之。
     *                  writeevent  表示添加的事件是否是写事件，1是，0反之。
     *                  otherflag   其他标记。
     *                  eventtype   事件类型，包括：增、删、改。
     *                  pconnsockinfo   连接池中对应的指针。
     * @return  0   操作成功。
     * @author  xuchanglong
     * @time    2019-08-27
    */
    int EpollAddEvent(const int &fd,
                      const int &readevent,
                      const int &writeevent,
                      const uint32_t &otherflag,
                      const uint32_t &eventtype,
                      XMNConnSockInfo *pconnsockinfo);

    /**
     * @function    epoll 等待接收和处理事件。
     * @paras   timer   等待事件的超时时间，单位 ms 。
     * @return  0   操作成功。
     * @author  xuchanglong
     * @time    2019-08-28
    */
    int EpollProcessEvents(int timer);

    /**
     * @function    返回消息链表中元素的数量。
     * @paras   none 。
     * @return   消息链表中元素的数量。
     * @author  xuchanglong
     * @time    2019-09-06
    */
    size_t RecvMsgListSize();

private:
    /**
     * @function    读取配置文件中的内容。
     * @paras   0   操作成功。
     *                  1  读取 port 数量失败。
     *                  2  读取 各个port 失败。
     * @author  xuchanglong
     * @time    2019-08-26
    */
    int ReadConf();

    /**
     *  @function    打开指定数量的监听 socket 并进行相关配置。
     *  @paras  pport  要监听的端口号的数组。
     *                  listenportcount   监听端口号的数量。
     *  @return 0   操作成功。
     *                     -1  sokcet 创建失败。
     *                  -2  setsockopt 设置失败。
     *                  -3  SetNonBlocking 设置失败。
     *                  -4  bind 绑定失败。
     *                  -5  listen  监听失败。
     *  @time   2019-08-25
    */
    int OpenListenSocket(const int *const pport, const size_t &listenportcount);

    /**
     *  @function   关闭监听 socket 。
     *  @paras  none 。
     *  @return 0   操作成功。
     *  @time   2019-08-25
    */
    int CloseListenSocket();

    /**
     *  @function    设置文件 IO 为非堵塞。
     *  @paras  sockfd  被设置的 IO 的文件描述符。
     *  @return 0   操作成功。
     *  @time   2019-08-25
    */
    int SetNonBlocking(const int &sockfd);

    //------------------------------------------ 业务处理 handler 。
    /**
     * @function    新的连接专用的处理函数。当连接进入时，该函数会被 EpollProcessEvents 所调用。
     * @paras   pconnsockinfo   连接池中的节点，该节点绑定了监听 socket 。
     * @return  none .
     * @author  xuchanglong
     * @time    2019-08-27
    */
    void EventAcceptHandler(XMNConnSockInfo *pconnsockinfo);

    /**
     * @function    关闭已经建立的连接。
     * @paras   pfd 待关闭的连接。
     * @return  none 。
     * @author  xuchanglong
     * @time    2019-08-29
    */
    void CloseConnection(XMNConnSockInfo *pfd);

    /**
     * @function    设置数据来时读处理的函数。
    */
    void WaitRequestHandler(XMNConnSockInfo *pconnsockinfo);

    /**
     * @function    从连接池中取出一个连接，该连接与 fd 对应的 socket 进行绑定。
     * @paras   fd  待绑定的 socket 。
     * @return  非0 绑定好的连接。
     *                  nullptr 池已空。
     * @author  xuchanglong
     * @time    2019-08-26
    */
    XMNConnSockInfo *GetConnSockInfo(const int &fd);

    /**
     * @function    回收指定的连接至连接池中。
     * @paras   pconnsockinfo   待回收的连接。
     * @return  none 。
     * @author  xuchanglong
     * @time    2019-08-29
    */
    void FreeConnSockInfo(XMNConnSockInfo *pconnsockinfo);

    /**
     * @function    从指定的连接中接收 bufflen 字节的数据到 pbuff 中。
     * @paras   pconnsockinfo   待接收数据的连接。
     *                  pbuff   保存接收到的数据。
     *                  bufflen 接收的数据的字节数。
     * @author  xuchanglong
     * @time    2019-08-31
    */
    ssize_t RecvData(XMNConnSockInfo *pconnsockinfo, char *pbuff, const size_t &bufflen);

    /**
     * @function    判断该包是否正常以及为接收包体做准备。
     * @paras   pconnsokcinfo   待处理的连接。
     * @return  none 。
     * @author  xuchanglong
     * @time    2019-08-31
    */
    void WaitRequestHandlerHeader(XMNConnSockInfo *pconnsokcinfo);

    /**
     * @function    对接收的完整的包进行处理（压入消息队列中并初始化状态机）。
     * @paras   pconnsokcinfo   待处理的连接。
     * @return  none 。
     * @author  xuchanglong
     * @time    2019-09-01
    */
    void WaitRequestHandlerBody(XMNConnSockInfo *pconnsokcinfo);

private:
    /**
     *  监听的 port 的数量。
    */
    int listenport_count_;

    /**
     * 保存待监听的 port 。
    */
    int *pportsum_;

    /**
     * 监听的 port 以及其对应的监听 socket 的 vector。
    */
    std::vector<XMNListenSockInfo *> vlistenportsockinfolist_;

    /**
     * 每个 worker 进程的 epoll 连接的最大项数。
    */
    int worker_connection_count_;

    /**
     * epoll 对象的文件描述符。
    */
    int epoll_handle_;

    /**
     * 连接池首地址。
    */
    XMNConnSockInfo *pconnsock_pool_;

    /**
     * 连接池中空闲连接链的表头。
    */
    XMNConnSockInfo *pfree_connsock_list_head_;

    /**
     * 连接池中所有连接的数量。
    */
    size_t pool_connsock_count_;

    /**
     * 连接池中可用连接的数量。
    */
    size_t pool_free_connsock_count_;

    /**
     * 用于存储 epoll_wait() 返回的发生的事件。
    */
    struct epoll_event wait_events_[XMN_EPOLL_WAIT_MAX_EVENTS];

    /**
     * 包头的大小。
    */
    size_t pkgheaderlen_;
    
    /**
     * 消息头的大小。
    */
    size_t msgheaderlen_;
};

#endif