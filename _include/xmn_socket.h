/*****************************************************************************************
 *  @function linux socket 配置文件。
 *  @author xuchanglong
 *  @time   2019-08-25
 *****************************************************************************************/
#ifndef XMOON__INCLUDE_XMN_SOCKET_H_
#define XMOON__INCLUDE_XMN_SOCKET_H_

#include <vector>
#include <cstddef>

/**
 * 存放已经完成连接的 socket 的队列的大小。
*/
#define XMN_LISTEN_BACKLOG 511

/**
 *  @function   存放监听端口号和其对应的监听socket的信息。
 *  @author xuchanglong
 *  @time   2019-08-25
*/
struct XMNListenPortSockInfo
{
    /**
     * 监听端口号。
    */
    size_t port;

    /**
     * 监听 socket 。
    */
    int fd;
};

class XMNSocket
{
public:
    XMNSocket();
    virtual ~XMNSocket();

public:
    /**
     *  @function   按照配置文件创建指定数量的监听 socket 。
     *  @paras  none 。
     *  @return 0   操作成功。
     *                    -1    配置文件中端口号数量 <= 0 。
     *  @time   2019-08-25
    */
    virtual int Initialize();

public:
    /**
     *  @function   关闭监听 socket 。
     *  @paras  none 。
     *  @return 0   操作成功。
     *  @time   2019-08-25
    */
    int CloseListeningSocket();

private:
    /**
     *  @function    打开指定数量的监听 socket 并进行相关配置。
     *  @paras  pport  要监听的端口号的数组。
     *                  listenportcount   监听端口号的数量。
     *  @return 0   操作成功。
     *  @time   2019-08-25
    */
    int OpenListenSocket(const int *const pport, const size_t &listenportcount);

    /**
     *  @function    设置文件 IO 为非堵塞。
     *  @paras  sockfd  被设置的 IO 的文件描述符。
     *  @return 0   操作成功。
     *  @time   2019-08-25
    */
    int SetNonBlocking(const int &sockfd);

private:
    /**
     *  监听的 port 的数量。
    */
    int listenport_count_;

    /**
     * 监听的 port 以及其对应的监听 socket 的链表。
    */
    std::vector<XMNListenPortSockInfo *> vlistenportsockinfolist_;
};

#endif