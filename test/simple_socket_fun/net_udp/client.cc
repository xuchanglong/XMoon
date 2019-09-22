#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <sys/types.h>
#include <arpa/inet.h>

#define PORT 59002
#define MAXDATASIZE 100

int main(int argc, char *argv[])
{
    /**
     * （1）变量初始化。
    */
    int clientfd, num;
    char buf[MAXDATASIZE];
    struct hostent *phostinfo;
    struct sockaddr_in serveraddr, peeraddr;
    memset(&serveraddr, 0, sizeof(struct sockaddr_in));
    memset(&peeraddr, 0, sizeof(struct sockaddr_in));
    if (argc != 3)
    {
        std::cout << "缺少 ip 和 port 。" << std::endl;
        return 1;
    }

    if ((phostinfo = gethostbyname(argv[1])) == nullptr)
    {
        printf("gethostbyname() error.");
        return 2;
    }

    /**
     * （2）创建 socket 。
    */
    clientfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientfd == -1)
    {
        perror("Creating socket failed.");
        return 1;
    }

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(PORT);
    serveraddr.sin_addr = *((struct in_addr *)phostinfo->h_addr_list[0]);

    socklen_t socklen = sizeof(struct sockaddr_in);
    while (true)
    {
        /**
         * （3）向指定 ip 和 port 的服务器发送用户指定的数据。
        */
        memset(buf, '\0', MAXDATASIZE);
        std::cin >> buf;
        sendto(clientfd, buf, MAXDATASIZE, 0, (struct sockaddr *)&serveraddr, sizeof(struct sockaddr_in));

        /**
         * （4）从服务器端接收数据从获得服务器的 ip 和 port 。
        */
        if ((num = recvfrom(clientfd, buf, MAXDATASIZE, 0, (struct sockaddr *)&peeraddr, &socklen)) == -1)
        {
            std::cout << "recvfrom() error." << std::endl;
            return 2;
        }

        /**
         * （5）判断前后两次的服务器是否是同一个。
        */
        if (socklen != sizeof(struct sockaddr_in) || memcmp((const void *)&serveraddr, (const void *)&peeraddr, socklen) != 0)
        {
            std::cout << "Receive message from other server.\n"
                      << std::endl;
            continue;
        }

        /**
         * （6）显示和判断从服务器上接收到的数据。
        */
        buf[num] = '\0';
        char serverip[20];
        memset(serverip, '\0', 20);
        inet_ntop(AF_INET, (const void *)&peeraddr.sin_addr.s_addr, serverip, 20);

        std::cout << "message is : " << buf << std::endl;
        std::cout << "server ip :" << serverip << std::endl;
        std::cout << "server port :" << ntohs(peeraddr.sin_port) << std::endl;
        std::cout << std::endl;

        if (!strcmp(buf, "bye"))
        {
            break;
        }
    }
    close(clientfd);
    return 0;
}