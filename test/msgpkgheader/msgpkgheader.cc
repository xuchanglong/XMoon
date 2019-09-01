#include "xmn_comm.h"
#include "unistd.h"

#include <sys/socket.h>
#include "sys/types.h"
#include "arpa/inet.h"
#include <iostream>
#include <cstring>

#define SERVERIP "192.168.1.105"
#define SERVERPORT 59002

struct LoginInfo
{
    char username[100];
    char password[100];
};

int connectserver(int &clientfd, const std::string &strserverip, const size_t &port);
void senddata(int clientfd, char *buf, const size_t &buflen);

int main()
{
    int clientfd = 0;
    int r = connectserver(clientfd, SERVERIP, SERVERPORT);
    if (r == 0)
    {
        std::cout << "连接 server 成功！" << std::endl;
    }
    else
    {
        std::cout << "连接 server 失败！" << std::endl;
    }

    int pkgheaderlen = sizeof(XMNPkgHeader);
    int loginfolen = sizeof(LoginInfo);
    char *sendbuf = new char[pkgheaderlen + loginfolen];

    XMNPkgHeader *ppkgheaser = (XMNPkgHeader *)sendbuf;
    ppkgheaser->pkglen = htons(pkgheaderlen + loginfolen);
    ppkgheaser->msgCode = htons(1);
    ppkgheaser->crc32 = htonl(123);

    LoginInfo *ploginfo = (LoginInfo *)(sendbuf + pkgheaderlen);
    std::strcpy(ploginfo->username, "xuchanglong");
    std::strcpy(ploginfo->password, "123456");
    while (true)
    {
        senddata(clientfd, sendbuf, pkgheaderlen + loginfolen);
        sleep(3);
    }
    return 0;
}

int connectserver(int &clientfd, const std::string &strserverip, const size_t &port)
{
    int r = 0;
    /**
     * TODO：这里需要添加第 3 个参数普遍选择 IPPROTO_IP 的原因。
    */
    /**
     * 创建连接 socket 。
    */
    clientfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TP);
    /**
     * 设置收发的超时时间。
    */
    int sendtime = 5000, rcvtime = 5000;
    if (setsockopt(clientfd, SOL_SOCKET, SO_SNDTIMEO, (void *)&sendtime, sizeof(int)) != 0)
    {
        std::cout << "setsockopt sendtime failed." << std::endl;
        return 2;
    }
    if (setsockopt(clientfd, SOL_SOCKET, SO_RCVTIMEO, (void *)&rcvtime, sizeof(int)) != 0)
    {
        std::cout << "setsockopt rcvtime failed." << std::endl;
        return 3;
    }
    /**
     * 连接 server 。
    */
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    serveraddr.sin_addr.s_addr = inet_addr(strserverip.c_str());
    r = connect(clientfd, (struct sockaddr *)&serveraddr, sizeof(struct sockaddr));
    if (r == -1)
    {
        std::cout << "Connected the server failed." << std::endl;
        return 1;
    }

    return 0;
}

void senddata(int clientfd, char *buf, const size_t &buflen)
{
    /**
     * 已发送的数据。
    */
    int uwrote = 0;
    int lentmp = 0;
    while (uwrote < buflen)
    {
        lentmp = send(clientfd, buf + uwrote, buflen - uwrote, 0);
        if (lentmp <= 0)
        {
            return;
        }
        uwrote += lentmp;
    }
    return;
}