/**
 * @function    简单的服务器的实现。
 * @author       xuchanglong
 * @time            2019-08-18
*/
/**
 * socket()、bind()
*/
#include "sys/types.h"
#include "sys/socket.h"
/**
 * IPPROTO_IP
*/
#include "linux/in.h"
/**
 * htonl()、htons()
*/
#include "arpa/inet.h"
/**
 * memset()
*/
#include "string.h"

#define SERVERPORT 59002
#define CLIENTSUM 5
/**
     * int socket (int __domain, int __type, int __protocol)
     *  
     * @function    打开socket，类似于文件操作的open函数。
     * @paras          __domain 协议族，AF_INET：IPV4
     *                                                                  AF_INET6：IPV6
     *                                                                  AF_UNIX
     *                                                                  AF_ROUTE。
     *                          __type      socket 类型，SOCK_STREAM：流式套接字
     *                                                                          SOCK_DGRAM：数据报套接字
     *                                                                          SOCK_RAW：原始套接字
     *                                                                          SOCK_PACKET
     *                                                                          SOCK_SEQPACKET
     *                          __protocol  协议类型，用来指明要接收的协议包。
     *                                                                          IPPROTO_IP（0），接收所有的数据包。
     *                                                                          IPPROTO_RAW（255），只发送不接受数据包。
     *                                                                          非 IPPROTO_IP 和 IPPROTO_RAW 的，例如 IPPROTO_TCP，仅仅接受TCP的数据包。
     * @return  socket 描述符（socket descriptor），和文件描述符一样。
    */

/**
    * int bind (int __fd, const struct sockaddr * __addr, socklen_t __len)
    * 
    * @function 为指定的 socket 绑定IP地址和port。
    * @paras    __fd    socket
    *                    __addr 指向要向 socket 绑定IP和port的地址。
    *                   const struct sockaddr   sin_family，协议族。与 socket 函数第1个参数一样。
    *                                                                   sin_port，绑定在 socket 上的port，该port是网络字节排序，需要用 htonl 来转换。l 是 long int 的意思。
    *                                                                   sin_addr.s_addr，IP 地址，网络字节排序，用 htons 来转换。s 是 short int 的意思。
    *                                                                                                           INADDR_ANY  0,0,0,0 地址，相当于本地所有地址的意思。
    *                                                                                                                                                                     本机有多块网卡，所有的网卡发来的数据都发送到该 socket 上。
    *                    __len  __addr 指向的地址的长度。
    * @return  后续补充。
   */

/**
 *  int listen (int __fd, int __n)
 * 
 * @function    监听由 __fd 发来的连接。socket()建立的socket是主动型的，该函数之后，该socket变成被动型。
 * @paras           __fd    要监听的套接字。
 *                           __n      最大的排队等待连接的数量。
 * @return        0     操作成功。
 *                          -1    失败。
*/

/**
 * int accept (int __fd, struct sockaddr * __addr, socklen_t * __addr_len)
 * @function    接收 client 发来的请求。
 * @paras           __fd            监控的socket。
 *                            __addr      存放客户端的 ip 和 port 信息。
 *                           __addr_len     上述存放客户端信息的结构体的大小
 * @return         已连接完成的socket。
*/

int main()
{
    /**
     * 其他变量初始化。
    */
    int r = 0;
    //保存连接的客户端的ip和port信息。
    struct sockaddr_in addrclient;
    memset(&addrclient, 0, sizeof(struct sockaddr_in) * 1);
    //保存连接的客户端的的ip和port信息的大小。
    socklen_t addrclientlen = 0;
    /**
     * 创建 socket 。
    */
    int listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    /**
     * 绑定IP和port。
   */
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in) * 1);
    addr.sin_family = AF_INET;
    addr.sin_port = htonl(SERVERPORT);
    addr.sin_addr.s_addr = htons(INADDR_ANY);
    int r = bind(listenfd, (struct sockaddr *)&addr, sizeof(addr));
    /**
     * 设置监听的client的数量。
    */
    r = listen(listenfd, CLIENTSUM);
    /**
     * 开始接收客户端发来的连接，并发送数据。
    */
    while (true)
    {
        r = accept(listenfd, (struct sockaddr *)&addrclient, &addrclientlen);
    }

    return 0;
}