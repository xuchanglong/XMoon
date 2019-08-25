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
     *                    1    配置文件中端口号数量 <= 0 。
     *                     -1  sokcet 创建失败。
     *                  -2  setsockopt 设置失败。
     *                  -3  SetNonBlocking 设置失败。
     *                  -4  bind 绑定失败。
     *                  -5  listen  监听失败。
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
     *                     -1  sokcet 创建失败。
     *                  -2  setsockopt 设置失败。
     *                  -3  SetNonBlocking 设置失败。
     *                  -4  bind 绑定失败。
     *                  -5  listen  监听失败。
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

    /**
     * @function    读取配置文件中的内容。
     * @paras   0   操作成功。
     *                  1  读取 port 数量失败。
     *                  2  读取 各个port 失败。
     * @author  xuchanglong
     * @time    2019-08-26
    */
    int ReadConf();

private:
    /**
     *  监听的 port 的数量。
    */
    int listenport_count_;

    /**
     * 保存待监听的 port 。
    */
    int *pportsum;

    /**
     * 监听的 port 以及其对应的监听 socket 的 vector。
    */
    std::vector<XMNListenPortSockInfo *> vlistenportsockinfolist_;

    /**
     * 每个 worker 进程的 epoll 连接的最大项数。
    */
    int worker_connection_count;
};

#endif