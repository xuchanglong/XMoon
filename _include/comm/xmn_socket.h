/*****************************************************************************************
 * 
 *  @function linux socket 配置文件。
 *  @author xuchanglong
 *  @time   2019-08-25
 * 
 *****************************************************************************************/
#ifndef XMOON__INCLUDE_XMN_SOCKET_H_
#define XMOON__INCLUDE_XMN_SOCKET_H_

#include "base/noncopyable.h"
#include "comm/xmn_socket_comm.h"

#include <cstddef>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <semaphore.h>

#include <atomic>
#include <vector>
#include <list>
#include <queue>
#include <map>

using CXMNSocket = class XMNSocket;
using XMNEventHandler = void (CXMNSocket::*)(struct XMNConnSockInfo *pconnsockinfo);

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
public:
    XMNConnSockInfo();
    ~XMNConnSockInfo();

public:
    /**
     * 将该连接的状态恢复至初始状态。
    */
    void InitConnSockInfo();

    /**
     * 释放连接关联的资源的内存。
    */
    void ClearConnSockInfo();

public:
    /**
     * 指向下一个该类型的对象。
     * 暂时无用。
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
     * 暂时无用。
    */
    unsigned int instance : 1;

    /**
     * 序号。
    */
    uint64_t currsequence;

    /**
     * 保存 client 地址信息用的。
    */
    struct sockaddr clientsockaddrinfo;

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
    XMNEventHandler rhandler;

    /**
     * 写事件相关处理函数。
    */
    XMNEventHandler whandler;

    /**
     * 存储该连接对应的 accept 返回的 socket 的触发事件类型。
     * 包括 EPOLLIN、EPOLLRDHUP 等。 
    */
    uint32_t events;

    /**
     * 业务逻辑处理的互斥量。
    */
    pthread_mutex_t logicprocmutex;

    /**************************************************************************************
     * 
     ***************** 与收包相关的变量 **************** 
     * 
    **************************************************************************************/
    /**
     * 接收当前数据包的状态。
    */
    RecvStatus recvstatus;

    /**
     * 存放包头数据。
    */
    char dataheader[XMN_PKG_HEADER_SIZE];

    /**
     * 存放当前需要接收的数据的首地址，供 RecvData 函数使用。
    */
    char *precvdatastart;

    /**
     * 当前需要接收的数据的字节数。
    */
    size_t recvdatalen;

    /**
     * 存放最终需要接收的所有数据，即：消息头 + 包头 + 包体。
    */
    char *precvalldata;

    /**
     * 标记 precvalldata 是否需要释放。
    */
    bool isfree;

    /**************************************************************************************
     * 
     ***************** 与发包相关的变量 *****************
     * 
    **************************************************************************************/
    /**
     * 保存整个消息的头指针，即：消息头 + 包头 + 包体，用于释放消息。
    */
    char *psendalldataforfree;

    /**
     * 保存最终需要发送的所有数据，即：包头 + 包体。
    */
    char *psenddata;

    /**
     * 待发送的数据的字节数量，即：psenddata（包头 + 包体）。
    */
    size_t senddatalen;

    /**
     * 记录该消息需要由 epoll_wait 来驱动发送的次数。
     * TODO：更准确的注释内容后续补充。
    */
    std::atomic<size_t> throwepollsendcount;

    /**************************************************************************************
     * 
     ***************** 和连接回收相关的变量 *****************
     * 
    **************************************************************************************/
    /**
     * 连接放入回收链表时的时间。
    */
    time_t putinrecylisttime;

    /**************************************************************************************
     * 
     ***************** 和心跳包相关的变量 *****************
     * 
    **************************************************************************************/
    /**
     * 最后一次接收到心跳包的时间。
    */
    time_t lastpingtime;

    /**************************************************************************************
     * 
     ***************** 与网络安全相关的变量 **************** 
     * 
    **************************************************************************************/
    /**
     * 上次受到 flood 攻击的时间。
    */
    uint64_t floodlasttime;

    /**
     * 一共连续受到 flood 攻击的次数。
    */
    std::atomic<size_t> floodattackcount;

    /**
     * 在发送消息队列中该连接对应的数据包的数量。
     * 用于防止某个 client 只发送不接收而导致服务器的问题。
     * 对于上述 client ，需要将其踢掉。
    */
    std::atomic<size_t> nosendmsgcount;
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
    /**
     * 保存单个线程的信息。
    */
    class ThreadInfo : public NonCopyable
    {
    public:
        ThreadInfo() = delete;
        ThreadInfo(XMNSocket *pthis) : pthis_(pthis)
        {
            isrunning_ = false;
            threadhandle_ = 0;
        }
        ~ThreadInfo(){};

    public:
        /**
         * 该线程所在的线程池的首地址。
        */
        XMNSocket *pthis_;
        /**
         * 该线程是否在运行。
        */
        bool isrunning_;
        /**
         * 该线程的描述符。
        */
        pthread_t threadhandle_;
    };

public:
    XMNSocket();
    virtual ~XMNSocket();

public:
    /**
     * @function   按照配置文件创建指定数量的监听 socket 。
     * @paras  none 。
     * @return  0   操作成功。
     *          1   配置文件中端口号数量 <= 0 。
     *          -1  sokcet 创建失败。
     *          -2  setsockopt 设置失败。
     *          -3  SetNonBlocking 设置失败。
     *          -4  bind 绑定失败。
     *          -5  listen  监听失败。
     * @author  xuchanglong
     * @time   2019-08-25
    */
    virtual int Initialize();

    /**
     * @function    部分内容需要在 worker 进程中初始化。
     * @paras   none 。
     * @return  0   操作成功。
     *          -1  connsock_pool_mutex_    初始化失败。
     *          -2  connsock_pool_recy_mutex_   初始化失败。
     * @author xuchanglong
     * @time    2019-09-21
    */
    virtual int InitializeWorker();

    /**
     * @function    worker 进程中初始化的内容需要在 worker 中释放。
     * @paras   none 。
     * @return  0 操作成功。
     * @author  xuchanglong
     * @time    2019-09-21
    */
    virtual int EndWorker();

    /**
     * @function    打印统计信息。
     * @paras   none 。
     * @return  none 。
     * @author  xuchanglong
     * @time    2019-10-07
    */
    void PrintInfo();

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
     *          readevent   表示添加的事件是否是读事件，1是，0反之。
     *          writeevent  表示添加的事件是否是写事件，1是，0反之。
     *          otherflag   其他标记。
     *          eventtype   事件类型，包括：增、删、改。
     *          pconnsockinfo   连接池中对应的指针。
     * @return  0   操作成功。
     * @author  xuchanglong
     * @time    2019-08-27
    */
    /*
    int EpollAddEvent(const int &fd,
                      const int &readevent,
                      const int &writeevent,
                      const uint32_t &otherflag,
                      const uint32_t &eventtype,
                      XMNConnSockInfo *pconnsockinfo);
    */

    /**
     * @function    向 epoll 中增加、删除、修改事件。
     * @paras   kSockFd 被 epoll 监控的 socket 。
     *          kOption 操作选项，包括：
     *                  增：EPOLL_CTL_ADD
     *                  删：EPOLL_CTL_DEL
     *                  改：EPOLL_CTL_MOD
     *          kEvents 事件类型。包括：
     *                  EPOLLIN：可读。
     *                  EPOLLRDHUP：TCP连接的远端关闭或者半关闭。
     *                  等。
     *          kFlag   为操作类型 EPOLL_CTL_MOD 补充操作：0：增加   1：去掉
     *          pconnsockinfo   该连接的信息。
     * @return  0   操作成功。
     * @author  xuchanglong
     * @time    2019-08-27
    */
    int EpollOperationEvent(const int &kSockFd,
                            const uint32_t &kOption,
                            const uint32_t &kEvents,
                            const int &kFlag,
                            XMNConnSockInfo *pconnsockinfo);

    /**
     * @function    epoll 等待接收和处理事件。
     * @paras   kTimer  等待事件的超时时间，单位 ms 。
     * @return  0   操作成功。
     * @author  xuchanglong
     * @time    2019-08-28
    */
    int EpollProcessEvents(const int &kTimer);

    /**
     * @function    返回消息链表中元素的数量。
     * @paras   none 。
     * @return   消息链表中元素的数量。
     * @author  xuchanglong
     * @time    2019-09-06
    */
    size_t RecvMsgListSize();

    /**************************************************************************************
     * 
     ***************** 线程相关操作 *****************
     * 
    **************************************************************************************/
    /**
     * @function    处理收到的数据包。
     * @paras   数据包。
     * @return  none 。
     * @author  xuchanglong
     * @time    2019-09-15
    */
    virtual void ThreadRecvProcFunc(char *pmsgbuf);

    /**************************************************************************************
     * 
     ***************** 与心跳监控相关的变量 *****************
     * 
    **************************************************************************************/
    virtual int PingTimeOutChecking(XMNMsgHeader *pmsgheader, time_t currenttime);

protected:
    /**
     * @function    发送数据。
     * @paras   psenddata   待发送的数据。
     * @return  0   操作成功。
     * @author  xuchanglong
     * @time    2019-09-25
    */
    int PutInSendDataQueue(char *psenddata);

    /**
     * @function    向 client 发送消息。
     * @paras   none 。
     * @return  > 0 发送成功，返回值就是已发送的数据的字节数。
     *          0   发送超时，对端已关闭。
     *              发送的数据本身是0个字节。
     *          -1  发送缓冲区已满。
     *          -2  未知错误。
     * @author  xuchanglong
     * @time    2019-09-26
    */
    int SendData(XMNConnSockInfo *pconnsockinfo);

    /**
     * @function    sever 端主动地关闭 socket 的函数。
     * @paras   pconnsockinfo   待关闭的连接信息。
     * @return  0   操作成功。
     *          -1  形参为空指针。
     * @author  xuchanglong
     * @time    2019-10-04
    */
    int ActivelyCloseSocket(XMNConnSockInfo *pconnsockinfo);

private:
    /**
     * @function    读取配置文件中的内容。
     * @paras   0  操作成功。
     *          1  读取 port 数量失败。
     *          2  读取 各个port 失败。
     * @author  xuchanglong
     * @time    2019-08-26
    */
    int ReadConf();

    /**
     *  @function    打开指定数量的监听 socket 并进行相关配置。
     *  @paras  none 。
     *  @return 0   操作成功。
     *          -1  sokcet 创建失败。
     *          -2  setsockopt 设置失败。
     *          -3  SetNonBlocking 设置失败。
     *          -4  bind 绑定失败。
     *          -5  listen  监听失败。
     *  @time   2019-08-25
    */
    int OpenListenSocket();

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
     * @function    由 epoll_wait 驱动，EpollProcessEvents 调用的函数。
     *              用于处理新建立的连接。
     * @paras   pconnsockinfo   连接池中的节点，该节点绑定了监听 socket 。
     * @return  none .
     * @author  xuchanglong
     * @time    2019-08-27
    */
    void EventAcceptHandler(XMNConnSockInfo *pconnsockinfo);

    /**
     * @function    由 epoll_wait 驱动，EpollProcessEvents 调用的函数。
     *              用于读取 client 发来的数据并做处理。
     * @paras   pconnsockinfo   连接池中的节点，该节点绑定了连接 socket 。
     * @return  none 。
     * @author  xuchanglong
     * @time    2019-09-26
    */
    void WaitReadRequestHandler(XMNConnSockInfo *pconnsockinfo);

    /**
     * @function    由 epoll_wait 驱动，EpollProcessEvents 调用的函数。
     *              用于向 client 发送数据。
     * @paras   pconnsockinfo   连接池中的节点，该节点绑定了连接 socket 。
     * @return  none 。
     * @author  xuchanglong
     * @time    2019-09-26
    */
    void WaitWriteRequestHandler(XMNConnSockInfo *pconnsockinfo);

    /**
     * @function    从指定的连接中接收 bufflen 字节的数据到 pbuff 中。
     * @paras   pconnsockinfo   待接收数据的连接。
     * @author  xuchanglong
     * @time    2019-08-31
    */
    ssize_t RecvData(XMNConnSockInfo *pconnsockinfo);

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

    /**************************************************************************************
     * 
     ***************** 和连接池相关的函数 *****************
     * 
    **************************************************************************************/
    /**
     * @function    初始化连接池。
     * @paras   none 。
     * @return  none 。
     * @author  xuchanglong
     * @time    2019-09-19
    */
    void InitConnSockInfoPool();

    /**
     * @function    释放连接池所占的内存。
     * @paras   none 。
     * @return  none 。
     * @author  xuchanglong
     * @time    2019-09-19
    */
    void FreeConnSockInfoPool();

    /**
     * @function    从连接池中取出一个连接，将 accept 返回的 socket 和该连接进行关联。
     * @paras   kSockFd accept 返回的 socket 。
     * @return  绑定好的连接池中的一个连接。
     *          nullptr 连接池中的连接为空。
     * @author  xuchanglong
     * @time    2019-09-19
    */
    XMNConnSockInfo *PutOutConnSockInfofromPool(const int &kSockFd);

    /**
     * @function    将连接归还至连接池中。
     * @paras   pconnsockinfo   待归还的连接。
     * @return  none 。
     * @author  xuchanglong
     * @time    2019-09-19
    */
    void PutInConnSockInfo2Pool(XMNConnSockInfo *pconnsockinfo);

    /**
     * @function    将连接放入回收链表中。
     * @paras   pconnsockinfo   待归还的连接。
     * @return  none 。
     * @author  xuchanglong
     * @time    2019-09-19
    */
    void PutInConnSockInfo2RecyList(XMNConnSockInfo *pconnsockinfo);

    /**
     * @function    定时将到时的连接归还至空闲连接池中。
     * @paras   pthreadinfo 线程的相关信息。
     * @return  nullptr .
     * @author  xuchanglong
     * @time    2019-09-19
    */
    static void *ConnSockInfoRecycleThread(void *pthreadinfo);

    /**
     * @function    关闭已经建立的连接。
     * @paras   pfd 待关闭的连接。
     * @return  none 。
     * @author  xuchanglong
     * @time    2019-08-29
    */
    void CloseConnection(XMNConnSockInfo *pfd);

    /**************************************************************************************
     * 
     ***************** 发送数据相关的函数 *****************
     * 
    **************************************************************************************/
    /**
     * @function    发送数据线程，每次循环只发送一次数据。
     * @paras   pthreadinfo   线程的相关信息。
     * @return  nullptr   操作成功。
     * @author  xuchanglong
     * @time    2019-09-25
    */
    static void *SendDataThread(void *pthreadinfo);

    /**
     * @function    从存储待发送的数据的数据池中获得消息。
     * @paras   none 。
     * @return  非 nullptr  待发送的消息。
     *          nullptr 数据池中不存在消息。
     * @author  xuchanglong
     * @time    2019-09-25
    */
    char *PutOutSendDataFromQueue();

    /**
     * @function    释放发送队列中的消息。
     * @paras   none 。
     * @return  0   操作成功。
     * @author  xuchanglong
     * @time    2019-09-26
    */
    int FreeSendDataQueue();

    /**************************************************************************************
     * 
     ***************** 与心跳监控相关的变量 **************** 
     * 
    **************************************************************************************/
    /**
     * @function    将指定的连接信息放入 multimap 中，等待心跳监控。
     * @paras   pconnsockinfo   指定的连接的信息。
     * @return  0   操作成功。
     * @author  xuchanglong
     * @time    2019-10-04
    */
    int PutInConnSockInfo2PingMultiMap(XMNConnSockInfo *pconnsockinfo);

    /**
     * @function    从 multimap 中获取最早的时间，即：头部元素。
     * @paras   none 。
     * @return  最早的时间。
     * @author  xuchanglong
     * @time    2019-10-04
    */
    time_t GetEarliestTime();

    /**
     * @function    对心跳包进行监控的执行线程。
     * @paras   pthreadinfo 该线程的相关信息。
     * @return  none 。
     * @author  xuchanglong
     * @time    2019-10-04
    */
    static void *PingThread(void *pthreadinfo);

    /**
     * @function    从监控 map 中找到比指定 time 最早的消息头。
     * @paras   待比较的时间。
     * @return  满足条件的消息。
     * @author  xuchanglong
     * @time    2019-10-04
    */
    XMNMsgHeader *GetOverTimeMsgHeader(const time_t &timetmp);

    /**
     * @function    从监控 map 中移除最早的时间并返回该消息。
     * @paras   none 。
     * @return  最早时间的消息。
     * @author  xuchanglong
     * @time    2019-10-04
    */
    XMNMsgHeader *RemoveFirstMsgHeader();

    /**
     * @function    从心跳监控的 multimap 中删除指定的消息头。
     * @paras   pconnsockinfo   待删除的连接。
     * @return  0   操作成功。
     *          -1  形参为空指针。
     * @author  xuchanglong
     * @time    2019-10-04
    */
    int PutOutMsgHeaderFromMultiMap(XMNConnSockInfo *pconnsockinfo);

    /**
     * @function    清空并释放心跳监控 multimap 。
     * @paras   none 。
     * @return  none 。
     * @author  xuchanglong
     * @time    2019-10-04
    */
    void FreePingMultiMap();

    /**************************************************************************************
     * 
     ***************** 与网络安全相关的函数 **************** 
     * 
    **************************************************************************************/
    /**
     * @function    测试当前连接对应的 client 是否存在恶意行为。
     * @paras   none 。
     * @return  true    存在恶意行为，应该关闭该连接。
     * @author  xuchanglong
     * @time    2019-10-06
    */
    bool TestFlood(XMNConnSockInfo *pconnsockinfo);

protected:
    /**
     * 消息头的大小。
    */
    const size_t kMsgHeaderLen_;

    /**
     * 包头的大小。
    */
    const size_t kPkgHeaderLen_;

    /**************************************************************************************
     * 
     ***************** 与心跳监控相关的变量 **************** 
     * 
    **************************************************************************************/
    /**
     * 心跳超时时间。
    */
    size_t pingwaittime_;

private:
    /**
     *  监听的 port 的数量。
    */
    size_t listenport_count_;

    /**
     * 保存待监听的 port 。
    */
    size_t *pportsum_;

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
     * 用于存储 epoll_wait() 返回的发生的事件。
    */
    struct epoll_event wait_events_[XMN_EPOLL_WAIT_MAX_EVENTS];

    /**
     * 保存每个 worker 进程专用的供 socket 类使用的线程的信息。
    */
    std::vector<ThreadInfo *> vthreadinfo_;

    /**************************************************************************************
     * 
     ***************** 与连接池相关的变量 **************** 
     * 
    **************************************************************************************/
    /**
     * 连接池列表。
    */
    std::list<XMNConnSockInfo *> connsock_pool_;

    /**
     * 连接池中所有连接的数量。
    */
    std::atomic<size_t> pool_connsock_count_;

    /**
     * 有关连接池操作的互斥量。
    */
    pthread_mutex_t connsock_pool_mutex_;

    /**
     * 空闲连接的列表。
    */
    std::list<XMNConnSockInfo *> connsock_pool_free_;

    /**
     * 空闲连接池中可用连接的数量。
    */
    std::atomic<size_t> pool_free_connsock_count_;

    /**
     * 待回收的连接的列表。
    */
    std::list<XMNConnSockInfo *> recyconnsock_pool_;

    /**
     * 待回收的连接的数量。
    */
    std::atomic<size_t> pool_recyconnsock_count_;

    /**
     * 有关回收连接到空闲列表操作的互斥量。
    */
    pthread_mutex_t connsock_pool_recy_mutex_;

    /**
     * 待回收的连接的等待时间。
    */
    int recyconnsockinfowaittime_;

    /**************************************************************************************
     * 
     ***************** 与发送消息相关的变量 **************** 
     * 
    **************************************************************************************/
    /**
     * 发送消息队列。
    */
    std::queue<char *> senddata_queue_;

    /**
     * 发送消息队列中消息的数量。
    */
    std::atomic<size_t> queue_senddata_count_;

    /**
     * 有关发送消息队列的相关操作的互斥量。
    */
    pthread_mutex_t senddata_queue_mutex_;

    /**
     * 与发送消息操作相关的信号量。
    */
    sem_t senddata_queue_sem_;

    /**************************************************************************************
     * 
     ***************** 与心跳监控相关的变量 **************** 
     * 
    **************************************************************************************/
    /**
     * 标记是否开启心跳包功能。
    */
    bool pingenable_;

    /**
     * 与心跳监控的 multimap 相关的互斥量。
    */
    pthread_mutex_t ping_multimap_mutex_;

    /**
     * 被心跳监控的连接信息的 multimap 。
    */
    std::multimap<time_t, XMNMsgHeader *> ping_multimap_;

    /**
     * ping_multimap_ 元素个数。
    */
    std::atomic<size_t> ping_multimap_count_;

    /**
     * ping_multimap_ 中最早时间的值。
    */
    time_t ping_multimap_headtime_;

    /**************************************************************************************
     * 
     ***************** 与在线用户数量相关的变量 **************** 
     * 
    **************************************************************************************/
    std::atomic<size_t> onlineuser_count_;

    /**************************************************************************************
     * 
     ***************** 与网络安全相关的变量 **************** 
     * 
    **************************************************************************************/
    /**
     * Flood 攻击检测是否开启的标志。
    */
    int floodattackmonitorenable_;

    /**
     * 相邻两次接收到数据包的最小时间间隔。
     * 小于该间隔被认为有 Flood 倾向。
     * 单位 ms 。
    */
    int floodtimeinterval_;

    /**
     * 连续发送若干次包，并且相邻两次的包的时间间隔小于 FloodTimeInterval 时，
     * 则认为该客户端是 Flood 的始作俑者。
    */
    int floodcount_;

    /**************************************************************************************
     * 
     ***************** 显示统计信息相关的变量 **************** 
     * 
    **************************************************************************************/
    /**
     * 上次在终端显示统计信息的时间。
    */
    time_t lastprinttime_;

    /**
     * 被丢弃的待发送的数据包的数量。
    */
    size_t discardsendpkgcount_;
};

#endif