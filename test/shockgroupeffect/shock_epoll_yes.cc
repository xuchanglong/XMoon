void worker(int sfd, int efd, struct epoll_event *events, int k) 
{
    /* The event loop */
    while (1)
    {
        int n, i;
        n = epoll_wait(efd, events, MAXEVENTS, -1);
        /*keep running*/
        sleep(2);
        printf("worker %d return from epoll_wait!\n", k);
        for (i = 0; i < n; i++)
        {
            if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (!(events[i].events & EPOLLIN)))
            {
                /* An error has occured on this fd, or the socket is not ready for reading (why were we notified then?) */
                fprintf(stderr, "epoll error\n");
                close(events[i].data.fd);
                continue;
            }
            else if (sfd == events[i].data.fd)
            {
                /* We have a notification on the listening socket, which means one or more incoming connections. */
                struct sockaddr in_addr;
                socklen_t in_len;
                int infd;
                char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
                in_len = sizeof in_addr;
                infd = accept(sfd, &in_addr, &in_len);
                if (infd == -1)
                {
                    printf("worker %d accept failed,error:%s\n", k, strerror(errno));
                    break;
                }
                printf("worker %d accept successed!\n", k);
                /* Make the incoming socket non-blocking and add it to the list of fds to monitor. */
                close(infd);
            }
        }
    }
}