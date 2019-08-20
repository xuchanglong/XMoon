/**
 * @function    简单的客户端的实现。
 * @author      xuchanglong
 * @time        2019-08-18
 * @website     https://www.cnblogs.com/jiangzhaowei/p/8261174.html
*/

#include "sys/types.h"
#include "sys/socket.h"
#include <iostream>
#include "unistd.h"
//#include "linux/in.h"
#include "arpa/inet.h"
#include <string.h>

#define SERVERPORT 59002

/**
 * int inet_pton(int af, const char *src, void *dst)
 * 
 * @function    将点分十进制转换为二进制整数。
 * @paras       af  协议族。
 *              src 点分十进制IP地址字符串。
 *              dst 转换完之后的二进制整数。
 * @return      > 0，操作成功。
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
 * @return      接收到的数据的数量。
*/

int main()
{
    int r = 0;
    /**
     * 保存从server端接收来的数据。
    */
    char buf[1000 + 1] = {'\0'};
    /**
     * 创建socket。
    */
    int clientfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    /**
     * 设置要连接的服务器的信息（ IP 和 port ）。
    */
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVERPORT);
    if (inet_pton(AF_INET, "192.168.1.105", &addr.sin_addr.s_addr) <= 0)
    {
        std::cout << "inet_pton failed." << std::endl;
        return 1;
    }
    /**
     * 连接 server 。
    */
    if (connect(clientfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        std::cout << "connect error,return value is " << r
                  << ",error code is " << errno
                  << ",error info is " << strerror(errno)
                  << "."<< std::endl;
        return 2;
    }
    /**
     * 读取 server 发来的数据。
    */
    if (read(clientfd, buf, 1000))
    {
        std::cout << "Recieve data is " << buf << std::endl;
        memset(buf, '\0', strlen(buf));
    }
    close(clientfd);
    std::cout << "程序执行完毕，退出！" << std::endl;
    return 0;
}