#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>

#define IP "127.0.0.1"
#define PORT 8888
#define PROCESS_NUM 4
#define MAXEVENTS 64

static int create_and_bind()
{
    int fd = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    inet_pton(AF_INET, IP, &serveraddr.sin_addr);
    serveraddr.sin_port = htons(PORT);
    bind(fd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    return fd;
}

static int make_socket_non_blocking(int sfd) 31
{
    int flags, s;
    flags = fcntl(sfd, F_GETFL, 0);
    if (flags == -1)
    {
        perror("fcntl");
        return -1;
    }
    flags |= O_NONBLOCK;
    s = fcntl(sfd, F_SETFL, flags);
    if (s == -1)
    {
        perror("fcntl");
        return -1;
    }
    return 0;
}

void worker(int sfd, int efd, struct epoll_event *events, int k)
{
    /* The event loop */
    while (1)
    {
        int n, i;
        n = epoll_wait(efd, events, MAXEVENTS, -1);
        printf("worker %d return from epoll_wait!\n", k);
        for (i = 0; i < n; i++)
        {
            if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (!(events[i].events & EPOLLIN)))
            {
                /* An error has occured on this fd, or the socket is not ready for reading (why were we notified then?) */ 56 fprintf(stderr, "epoll error\n");
                close(events[i].data.fd);
                continue;
            }
            else if (sfd == events[i].data.fd)
            {
                /* We have a notification on the listening socket, which means one or more incoming connections. */ 61 struct sockaddr in_addr;
                socklen_t in_len;
                int infd;
                char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
                in_len = sizeof in_addr;
                infd = accept(sfd, &in_addr, &in_len);
                if (infd == -1)
                {
                    printf("worker %d accept failed!\n", k);
                    break;
                }
                printf("worker %d accept successed!\n", k);
                /* Make the incoming socket non-blocking and add it to the list of fds to monitor. */ 73 close(infd);
            }
        }
    }
}

int main(int argc, char *argv[]) 80
{
    int sfd, s;
    int efd;
    struct epoll_event event;
    struct epoll_event *events;
    sfd = create_and_bind();
    if (sfd == -1)
    {
        abort();
    }
    s = make_socket_non_blocking(sfd);
    if (s == -1)
    {
        abort();
    }
    s = listen(sfd, SOMAXCONN);
    if (s == -1)
    {
        perror("listen");
        abort();
    }
    efd = epoll_create(MAXEVENTS);
    if (efd == -1)
    {
        perror("epoll_create");
        abort();
    }
    event.data.fd = sfd;
    event.events = EPOLLIN;
    s = epoll_ctl(efd, EPOLL_CTL_ADD, sfd, &event);
    if (s == -1)
    {
        perror("epoll_ctl");
        abort();
    }
    /* Buffer where events are returned */
    events = calloc(MAXEVENTS, sizeof event);
    int k;
    for (k = 0; k < PROCESS_NUM; k++)
    {
        printf("Create worker %d\n", k + 1);
        int pid = fork();
        if (pid == 0)
        {
            worker(sfd, efd, events, k);
        }
    }
    int status;
    wait(&status);
    free(events);
    close(sfd);
    return EXIT_SUCCESS;
}