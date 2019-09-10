#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <iostream>

#define PORT 59002
#define MAXDATASIZE 100

int main()
{
    /**
     * （1）变量的初始化。
    */
    int serverfd;
    struct sockaddr_in addr_server;
    struct sockaddr_in addr_client;
    socklen_t socklen;
    char buf[MAXDATASIZE];
    int num = 0;

    /**
     * （2）创建 socket 。
    */
    if ((serverfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("Creating socked failed.");
        return 1;
    }

    /**
     * （3） 绑定服务器端的 ip 和 port 。
    */
    addr_server.sin_family = AF_INET;
    addr_server.sin_port = ntohs(PORT);
    addr_server.sin_addr.s_addr = ntohl(INADDR_ANY);
    if ((bind(serverfd, (struct sockaddr *)&addr_server, sizeof(struct sockaddr_in))) == -1)
    {
        std::cout << "Bind() error." << std::endl;
        return 2;
    }
    socklen = sizeof(struct sockaddr_in);
    while (true)
    {
        /**
         * （4）接收客户端发来的数据以及客户端的 ip 和 port 。
        */
        num = recvfrom(serverfd, buf, MAXDATASIZE, 0, (struct sockaddr *)&addr_client, &socklen);
        if (num < 0)
        {
            std::cout << "recvfrom error." << std::endl;
            return 3;
        }
        buf[num] = '\0';
        char clientip[20];
        memset(clientip, '\0', 20);
        inet_ntop(AF_INET, (void *)&addr_client.sin_addr.s_addr, clientip, 20);

        std::cout << "message :" << buf << std::endl;
        std::cout << "client ip :" << clientip << std::endl;
        std::cout << "client port :" << ntohs(addr_client.sin_port) << std::endl;
        std::cout << std::endl;

        /**
         * （5）向指定 ip 和 port 的客户端发送数据。
        */
        sendto(serverfd, buf, num, 0, (struct sockaddr *)&addr_client, socklen);

        if (!strcmp(buf, "bye"))
        {
            break;
        }
    }

    return 0;
}