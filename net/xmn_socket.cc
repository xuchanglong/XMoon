#include "xmn_socket.h"
#include "xmn_config.h"
#include "xmn_func.h"
#include "xmn_macro.h"
#include "sys/socket.h"
#include "sys/types.h"
#include "sys/ioctl.h"
#include "linux/sockios.h"
#include "arpa/inet.h"
#include <cstdio>
#include "errno.h"
#include "unistd.h"
#include <sstream>

XMNSocket::XMNSocket()
{
    listenport_count_ = 0;
}

XMNSocket::~XMNSocket()
{
    std::vector<XMNListenPortSockInfo *>::iterator it;
    for (it = vlistenportsockinfolist_.begin(); it != vlistenportsockinfolist_.end(); ++it)
    {
        delete *it;
        *it = nullptr;
    }
    vlistenportsockinfolist_.clear();
}

int XMNSocket::Initialize()
{
    XMNConfig *pconfig = XMNConfig::GetInstance();
    /**
     * 获取共有多少个 port 。
    */
    listenport_count_ = atoi(pconfig->GetConfigItem("ListenPortCount").c_str());
    if (listenport_count_ <= 0)
    {
        return 1;
    }
    /**
     * 获取这些端口号。
    */
    int *pports = new int[listenport_count_];
    std::string str;
    std::stringstream s;
    for (size_t i = 0; i < listenport_count_; i++)
    {
        s << i;
        str = "ListenPort" + s.str();
        pports[i] = atoi(pconfig->GetConfigItem(str).c_str());
    }

    /**
     * 开启指定的端口号。
    */
    int r = OpenListenSocket(pports, listenport_count_);

    delete[] pports;
    pports = nullptr;
    return r;
}

int XMNSocket::OpenListenSocket(const int *const pport, const size_t &listenportcount)
{
    int exitcode = 0;
    int r = 0;

    /**
     * 存放创建的监听 socket 。
    */
    int *psocksum = new int[listenportcount]();

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    /**
     * 对每一个 port 创建一个 socket 。
    */
    for (size_t i = 0; i < listenportcount; i++)
    {
        /**
         *  创建连接 socket 。
        */
        psocksum[i] = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (psocksum[i] <= 0)
        {
            xmn_log_info(XMN_LOG_EMERG, errno, "OpenListenSocket create listen socket failed.");
            exitcode = -1;
            goto exitlabel;
        }

        /**
         * 设置 server 关闭之后可以立刻重启 server 的功能，即：地址重用功能。
        */
        int reuseaddr = 1;
        r = setsockopt(psocksum[i], SOL_SOCKET, SO_REUSEADDR, (const void *)&reuseaddr, sizeof(reuseaddr));
        if (r != 0)
        {
            xmn_log_info(XMN_LOG_EMERG, errno, "OpenListenSocket setsockopt failed.");
            exitcode = -2;
            goto exitlabel;
        }

        /**
         * 设置 socket 为非堵塞模式。
        */
        r = SetNonBlocking(psocksum[i]);
        if (r != 0)
        {
            xmn_log_info(XMN_LOG_EMERG, errno, "OpenListenSocket SetNonBlocking failed.");
            exitcode = -3;
            goto exitlabel;
        }

        /**
         * 绑定 IP 和 port 。
        */
        addr.sin_port = htons(pport[i]);
        r = bind(psocksum[i], (struct sockaddr *)&addr, sizeof(addr));
        if (r != 0)
        {
            xmn_log_info(XMN_LOG_EMERG, errno, "OpenListenSocket bind failed.");
            exitcode = -4;
            goto exitlabel;
        }

        /**
         * 开始监听。
        */
        r = listen(psocksum[i], XMN_LISTEN_BACKLOG);
        if (r != 0)
        {
            xmn_log_info(XMN_LOG_EMERG, errno, "OpenListenSocket listen failed.");
            exitcode = -5;
            goto exitlabel;
        }

        /**
         * 将 port 和 soket 插入 vector 中。
        */
        XMNListenPortSockInfo *pitem = new XMNListenPortSockInfo;
        pitem->fd = psocksum[i];
        pitem->port = pport[i];
        vlistenportsockinfolist_.push_back(pitem);

        xmn_log_info(XMN_LOG_INFO, 0, "监听端口 %d 的socket 创建成功！", pport[i]);
    }
    return 0;

exitlabel:
    for (size_t i = 0; i < listenportcount; i++)
    {
        close(psocksum[i]);
    }
    delete[] psocksum;
    psocksum = nullptr;
    return exitcode;
}

int XMNSocket::CloseListeningSocket()
{
    std::vector<XMNListenPortSockInfo *>::iterator it;
    for (it = vlistenportsockinfolist_.begin(); it != vlistenportsockinfolist_.end(); it++)
    {
        close((*it)->fd);
        xmn_log_info(XMN_LOG_INFO, 0, "监听端口 %d 的 socket 已经关闭！", (*it)->port);
    }
    return 0;
}

int XMNSocket::SetNonBlocking(const int &sockfd)
{
    int setnoblock = 1;
    return ioctl(sockfd, FIONBIO, &setnoblock);
}