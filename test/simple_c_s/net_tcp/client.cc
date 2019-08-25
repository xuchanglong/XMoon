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

void showerrorinfo(const std::string &strfun, const int &ireturnvalue, const int &err);

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
    if (clientfd <= 0)
    {
        showerrorinfo("socket", r, errno);
        return 1;
    }

    /**
     * 设置要连接的服务器的信息（ IP 和 port ）。
    */
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVERPORT);
    r = inet_pton(AF_INET, "192.168.1.105", &addr.sin_addr.s_addr);
    if (r <= 0)
    {
        showerrorinfo("inet_pton", r, errno);
        return 1;
    }

    /**
     * 连接 server 。
    */
    r = connect(clientfd, (struct sockaddr *)&addr, sizeof(addr));
    if (r != 0)
    {
        showerrorinfo("connect", r, errno);
        return 3;
    }

    /**
     * 读取 server 发来的数据。
    */
    if (read(clientfd, buf, 1000))
    {
        std::cout << "Recieve data is \"" << buf <<"\""<< std::endl;
        memset(buf, '\0', strlen(buf));
    }

    close(clientfd);
    std::cout << "程序执行完毕，退出！" << std::endl;
    return 0;
}

void showerrorinfo(const std::string &strfun, const int &ireturnvalue, const int &err)
{
    std::cout << strfun << " error,return value is \"" << ireturnvalue << "\""
              << ",error code is \"" << err << "\""
              << ",error info is \"" << strerror(err) << "\""
              << "." << std::endl;
}