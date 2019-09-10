#include <sys/socket.h>
#include <stdio.h>

int main()
{
    int clientfd;
    clientfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientfd == -1)
    {
        perror("Creating socket failed.");
        return 1;
    }

    return 0;
}