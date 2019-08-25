/**
 * @function    简单的服务器的实现。
 * @author      xuchanglong
 * @time        2019-08-18
 * @website     https://www.cnblogs.com/jiangzhaowei/p/8261174.html
 *                          https://blog.csdn.net/sandware/article/details/40923491
 *                          https://blog.csdn.net/xioahw/article/details/4056514
 * @notice  查看socket的状态的指令：netstat -anp | grep -E "State|59002"
*/

/**
 * socket()、bind()
*/
#include "sys/types.h"
#include "sys/socket.h"
/**
 * htonl()、htons()
*/
#include "arpa/inet.h"
/**
 * memset()
*/
#include "string.h"
/**
 * write()、read()
*/
#include "unistd.h"
#include <iostream>

/**
 * 端口号。
*/
#define SERVERPORT 59002
/**
 * 等待连接的客户端的最大的数量。
*/
#define CLIENTSUM 5

/**
 * @function    在终端显示函数返回的错误信息。
 * @paras           str     报错的函数。
 *                          ireturnvalue    函数的返回值。
 *                          err errno，即：错误代码。
 * @return      none。
 * @author      xuchanglong
 * @time           2019-08-24
*/
void showerrorinfo(const std::string &strfun, const int &ireturnvalue, const int &err);

int main()
{
    /**
     * 其他变量初始化。
    */
    int r = 0;
    int isenddatasum = 0;
    //保存连接的 client 的 ip 和 port 信息。
    struct sockaddr_in addrclient;
    memset(&addrclient, 0, sizeof(struct sockaddr_in) * 1);
    //保存连接的 client 的的 ip 和 port 信息的大小。
    socklen_t addrclientlen = 0;
    //要发送给 client 的数据。
    const char *pbuf = "Hello client.\n";
    //接受客户端连接而得到的连接套接字。
    int confd = 0;

    /**
     * 创建 socket 。
    */
    int listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (listenfd <= 0)
    {
        showerrorinfo("socket", r, errno);
        return 1;
    }

    /**
     * 开启 server 关闭时可以立刻重启 server 的功能。
    */
    int reuseaddr = 1;
    r = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&reuseaddr, sizeof(reuseaddr));
    if (r != 0)
    {
        showerrorinfo("setsockopt", r, errno);
        return 2;
    }

    /**
     * 绑定 IP 和 port 。
    */
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in) * 1);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVERPORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    r = bind(listenfd, (struct sockaddr *)&addr, sizeof(addr));
    if (r != 0)
    {
        showerrorinfo("bind", r, errno);
        return 3;
    }

    /**
     * 设置监听的 client 的数量。
    */
    r = listen(listenfd, CLIENTSUM);
    if (r != 0)
    {
        showerrorinfo("listen", r, errno);
        return 4;
    }
    else
    {
        std::cout << "Server start successfully and listening." << std::endl;
    }

    /**
     * 开始接收 client 发来的连接，并发送数据。
    */
    while (true)
    {
        /**
         * 接受 client 发来的连接请求，该函数正常返回时就意味着完成了 TCP 三次握手。
        */
        confd = accept(listenfd, (struct sockaddr *)&addrclient, &addrclientlen);

        /**
         * 向 client 发送数据。
        */
        isenddatasum = write(confd, pbuf, strlen(pbuf));

        std::cout << "Send \"" << isenddatasum << "\" bytes." << std::endl;
        close(confd);
    }
    close(listenfd);
    return 0;
}

void showerrorinfo(const std::string &strfun, const int &ireturnvalue, const int &err)
{
    std::cout << strfun << " error,return value is \"" << ireturnvalue << "\""
              << ",error code is \"" << err << "\""
              << ",error info is \"" << strerror(err) << "\""
              << "." << std::endl;
}