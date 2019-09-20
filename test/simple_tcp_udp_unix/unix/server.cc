/**
 * 参考链接：
 * https://www.jb51.net/article/127352.htm
 * https://blog.csdn.net/gladyoucame/article/details/8768731
*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#define UNIX_DOMAIN "/tmp/UNIX.domain"
int main(void)
{
    socklen_t clt_addr_len;
    int listen_fd;
    int com_fd;
    int ret;
    int i;
    static char recv_buf[1024];
    int len;
    struct sockaddr_un clt_addr;
    struct sockaddr_un srv_addr;
    listen_fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (listen_fd < 0)
    {
        perror("cannot create communication socket");
        return 1;
    }
    //set server addr_param
    srv_addr.sun_family = AF_UNIX;
    strncpy(srv_addr.sun_path, UNIX_DOMAIN, sizeof(srv_addr.sun_path) - 1);
    unlink(UNIX_DOMAIN);
    //bind sockfd & addr
    ret = bind(listen_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));
    if (ret == -1)
    {
        perror("cannot bind server socket");
        close(listen_fd);
        unlink(UNIX_DOMAIN);
        return 1;
    }
    //listen sockfd
    ret = listen(listen_fd, 1);
    if (ret == -1)
    {
        perror("cannot listen the client connect request");
        close(listen_fd);
        unlink(UNIX_DOMAIN);
        return 1;
    }
    //have connect request use accept
    len = sizeof(clt_addr);
    com_fd = accept(listen_fd, (struct sockaddr *)&clt_addr, &len);
    if (com_fd < 0)
    {
        perror("cannot accept client connect request");
        close(listen_fd);
        unlink(UNIX_DOMAIN);
        return 1;
    }
    //read and printf sent client info
    printf("/n=====info=====/n");
    for (i = 0; i < 4; i++)
    {
        memset(recv_buf, 0, 1024);
        int num = read(com_fd, recv_buf, sizeof(recv_buf));
        printf("Message from client (%d)) :%s/n", num, recv_buf);
    }
    close(com_fd);
    close(listen_fd);
    unlink(UNIX_DOMAIN);
    return 0;
}