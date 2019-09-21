#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <iostream>

#define UNIX_DOMAIN "/tmp/UNIX.domain"

int main(void)
{
    /**
     * （1）定义相关变量。
    */
    int linkfd = 0;
    int ret = 0;
    char sendbuf[1024];
    struct sockaddr_un addr_server;
    int exitcode = 0;

    /**
     * （2）创建连接 socket 。
    */
    linkfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (linkfd < 0)
    {
        std::cout << "cannot create communication socket" << std::endl;
        exitcode = -1;
        goto lblexit;
    }

    /**
     * （3）设置 server 端 sockaddr_un 。
    */
    addr_server.sun_family = AF_UNIX;
    memcpy(addr_server.sun_path, UNIX_DOMAIN, sizeof(UNIX_DOMAIN));

    /**
     * （4）连接 server 。 
    */
    ret = connect(linkfd, (struct sockaddr *)&addr_server, sizeof(addr_server));
    if (ret == -1)
    {
        std::cout << "cannot connect to the server" << std::endl;
        exitcode = -2;
        goto lblexit;
    }
    memset(sendbuf, 0, 1024);
    strcpy(sendbuf, "message from client");

    /**
     * （5）向 server 发送数据。 
    */
    for (size_t i = 0; i < 4; i++)
    {
        write(linkfd, sendbuf, sizeof(sendbuf));
    }

lblexit:
    close(linkfd);
    unlink(UNIX_DOMAIN);
    return exitcode;
}