#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>

#define PORT 59002
#define MAXDATASIZE 100

int main()
{
    int serverfd;
    struct sockaddr_in addr_server;
    struct sockaddr_in addr_client;
    socklen_t socklen;
    char buf[MAXDATASIZE];
    int num = 0;

    if ((serverfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("Creating socked failed.");
        return 1;
    }
    addr_server.sin_family = AF_INET;
    addr_server.sin_port = ntohs(PORT);
    addr_server.sin_addr.s_addr = ntohl(INADDR_ANY);
    if ((bind(serverfd, (struct sockaddr *)&addr_server, sizeof(struct sockaddr_in))) == -1)
    {
        perror("Bind() error.");
        return 2;
    }
    socklen = sizeof(struct sockaddr_in);
    while (true)
    {
        num = recvfrom(serverfd, buf, MAXDATASIZE, 0, (struct sockaddr *)&addr_client, &socklen);
        if (num < 0)
        {
            perror("recvfrom error.");
            return 3;
        }
        buf[num] = '\0';
        char clientip[20];
        memset(clientip, '\0', 20);
        inet_ntop(AF_INET, (void *)&addr_client.sin_addr.s_addr, clientip, 20);

        printf("You got a message <%s> from client.\nIt's ip is %s,port is %d.\n",
               buf, clientip, ntohs(addr_client.sin_port));
               
        sendto(serverfd, "Welcome\n", 8, 0, (struct sockaddr *)&addr_client, socklen);

        if (strcmp(buf, "bye"))
        {
            break;
        }
    }

    return 0;
}