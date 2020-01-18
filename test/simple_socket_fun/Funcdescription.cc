
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
 * @ret  socket 描述符（socket descriptor），和文件描述符一样（监听socket）。
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
 * @ret   0  操作成功。
 *           -1 操作失败，由全局变量 errno 来获取报错代码。
*/

/**
 *  int listen (int __fd, int __n)
 * 
 * @function    监听由 __fd 发来的连接。socket()建立的socket是主动型的，该函数之后，该socket变成被动型。
 * @paras       __fd    要监听的套接字。
 *              __n     最大的排队等待连接的数量。
 * @ret      0  操作成功。
 *              -1 操作失败，由全局变量 errno 来获取报错代码。
*/

/**
 * int accept (int __fd, struct sockaddr * __addr, socklen_t * __addr_len)
 * 
 * @function    接收 client 发来的请求。
 * @paras       __fd        监控的socket。
 *              __addr      存放 client 的 ip 和 port 信息。
 *              __addr_len  属于值-返回值，即：传入时必须是 struct sockaddr 的大小，
 *                                                                                  返回时，该值存放 __addr 实际的大小。
 * @ret      >0  连接socket，即：已连接完成的socket。
 *                         -1   有错误。
*/

/**
 * ssize_t write(int fd, const void *buf, size_t count);
 * 
 * @function    向 fd 写入（发送）数据。
 * @paras       fd        连接socket
 *              buf       待发送的数据。
 *              count     待发送的数据的字节数量。
 * @ret      > 0，发送的数据的数量。
 *              -1，发送失败，可通过 errno 来确定错误代码。
*/

/**
 * int inet_pton(int af, const char *src, void *dst)
 * 
 * @function    将点分十进制转换为二进制整数。
 * @paras       af  协议族。
 *              src 点分十进制IP地址字符串。
 *              dst 转换完之后的二进制整数。
 * @ret      > 0，操作成功。
 *              0，点分十进制IP地址和协议族不匹配。
 *              负值，报错。
*/

/**
 * size_t read(int fd, void *buf, size_t count);
 * 
 * @function    从 fd 读取（接收）数据。
 * @paras       fd        客户端 socket
 *              buf       接收到的数据的存放地址。
 *              count     buf 内存字节数大小。
 * @ret      接收到的数据的数量。
*/