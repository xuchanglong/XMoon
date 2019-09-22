/**
 * 参考链接：
 * https://www.jb51.net/article/127352.htm
 * https://blog.csdn.net/gladyoucame/article/details/8768731
*/
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
    int listen_fd = 0;
    int linkfd = 0;
    static char recv_buf[1024];
    socklen_t socklen = 0;
    struct sockaddr_un addr_server;
    struct sockaddr_un addr_client;
    int ret = 0;
    int exitcode = 0;

    /**
     * （2）创建连接 socket 。
    */
    listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (listen_fd < 0)
    {
        std::cout << "cannot create communication socket" << std::endl;
        exitcode = -1;
        goto lblexit;
    }

    /**
     * （3）设置 sockaddr_un 。
    */
    addr_server.sun_family = AF_UNIX;
    strcpy(addr_server.sun_path, UNIX_DOMAIN);
    /**
     * 执行unlink()函数并不一定会真正的删除文件，它先会检查文件系统中此文件的连接数是否为1，
     * 如果不是1说明此文件还有其他链接对象，因此只对此文件的连接数进行减1操作。
     * 若连接数为1，并且在此时没有任何进程打开该文件，此内容才会真正地被删除掉。
     * 在有进程打开此文件的情况下，则暂时不会删除，直到所有打开该文件的进程都结束时文件就会被删除。
    */
    unlink(UNIX_DOMAIN);

    /**
     * （4）连接 socket 和 sockaddr_un 绑定。
    */
    ret = bind(listen_fd, (struct sockaddr *)&addr_server, sizeof(sockaddr_un));
    if (ret == -1)
    {
        std::cout << "cannot bind server socket" << std::endl;
        exitcode = -2;
        goto lblexit;
    }

    /**
     * （5）将连接 socket 变为监听 socket 。
    */
    ret = listen(listen_fd, 5);
    if (ret == -1)
    {
        std::cout << "cannot listen the client connect request" << std::endl;
        exitcode = -3;
        goto lblexit;
    }

    /**
     * （6）开始接受 client 发来的连接。
    */
    socklen = sizeof(addr_client);
    linkfd = accept(listen_fd, (struct sockaddr *)&addr_client, &socklen);
    if (linkfd < 0)
    {
        std::cout << "cannot accept client connect request" << std::endl;
        exitcode = -4;
        goto lblexit;
    }

    for (size_t i = 0; i < 4; i++)
    {
        memset(recv_buf, 0, 1024);
        int num = read(linkfd, recv_buf, sizeof(recv_buf));
        std::cout << "Message from client is \"" << recv_buf << "\"" << std::endl;
    }

lblexit:
    close(linkfd);
    close(listen_fd);
    unlink(UNIX_DOMAIN);
    return exitcode;
}