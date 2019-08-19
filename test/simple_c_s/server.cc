/**
 * @function    简单的服务器的实现。
 * @author      xuchanglong
 * @time        2019-08-18
 * @website     https://www.cnblogs.com/jiangzhaowei/p/8261174.html
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
 * int socket (int __domain, int __type, int __protocol)
 *  
 * @function    打开socket，类似于文件操作的open函数。
 * @paras          __domain 协议族，AF_INET：IPV4
 *                                 AF_INET6：IPV6
 *                                 AF_UNIX
 *                                 AF_ROUTE。
 *                 __type      socket 类型，SOCK_STREAM：流式套接字
 *                                         SOCK_DGRAM：数据报套接字
 *                                         SOCK_RAW：原始套接字
 *                                         SOCK_PACKET
 *                                         SOCK_SEQPACKET
 *                 __protocol  协议类型，用来指明要接收的协议包。
 *                             IPPROTO_IP（0），接收所有的数据包。
 *                             IPPROTO_RAW（255），只发送不接受数据包。
 *                             非 IPPROTO_IP 和 IPPROTO_RAW 的，例如 IPPROTO_TCP，仅仅接受TCP的数据包。
 * @return  socket 描述符（socket descriptor），和文件描述符一样。
*/

/**
 * int bind (int __fd, const struct sockaddr * __addr, socklen_t __len)
 * 
 * @function 为指定的 socket 绑定IP地址和port。
 * @paras    __fd    socket
 *           __addr 指向要向 socket 绑定IP和port的地址。
 *           const struct sockaddr   sin_family，协议族。与 socket 函数第1个参数一样。
 *                                   sin_port，绑定在 socket 上的port，该port是网络字节排序，需要用 htonl 来转换。l 是 long int 的意思。
 *                                   sin_addr.s_addr，IP 地址，网络字节排序，用 htons 来转换。s 是 short int 的意思。
 *                                                    INADDR_ANY  0,0,0,0 地址，相当于本地所有地址的意思。
 *                                                                                                                                                                     本机有多块网卡，所有的网卡发来的数据都发送到该 socket 上。
 *           __len  __addr 指向的地址的长度。
 * @return  后续补充。
*/

/**
 *  int listen (int __fd, int __n)
 * 
 * @function    监听由 __fd 发来的连接。socket()建立的socket是主动型的，该函数之后，该socket变成被动型。
 * @paras       __fd    要监听的套接字。
 *              __n     最大的排队等待连接的数量。
 * @return      0       操作成功。
 *              -1      失败。
*/

/**
 * int accept (int __fd, struct sockaddr * __addr, socklen_t * __addr_len)
 * 
 * @function    接收 client 发来的请求。
 * @paras       __fd        监控的socket。
 *              __addr      存放 client 的 ip 和 port 信息。
 *              __addr_len  上述存放 client 信息的结构体的大小
 * @return      连接socket，即：已连接完成的socket。
*/

/**
 * ssize_t write(int fd, const void *buf, size_t count);
 * 
 * @function    向 fd 写入（发送）数据。
 * @paras       fd        连接socket
 *              buf       待发送的数据。
 *              count     待发送的数据的字节数量。
 * @return      > 0，发送的数据的数量。
 *              -1，发送失败，可通过 errno 来确定错误代码。
*/

int main()
{
    /**
     * 其他变量初始化。
    */
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
    /**
     * 绑定 IP 和 port 。
    */
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in) * 1);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVERPORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(listenfd, (struct sockaddr *)&addr, sizeof(addr));
    /**
     * 设置监听的 client 的数量。
    */
    if (listen(listenfd, CLIENTSUM) != 0)
    {
        std::cout << "listen failed!" << std::endl;
        return 2;
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
        write(confd, pbuf, strlen(pbuf));
        close(confd);
    }
    close(listenfd);
    return 0;
}