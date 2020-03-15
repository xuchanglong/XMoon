#include "comm/xmn_socket_comm.h"
#include "comm/xmn_socket_logic_comm.h"
#include "xmn_crc32.h"

#include "unistd.h"
#include <sys/socket.h>
#include "sys/types.h"
#include "arpa/inet.h"
#include <iostream>
#include <cstring>

#define SERVERIP "127.0.0.1"
#define SERVERPORT 80

int connectserver(int &clientfd, const std::string &strserverip, const size_t &port);
void senddata(int clientfd, char *buf, const size_t &buflen);
void showerrorinfo(const std::string &strfun, const int &ireturnvalue, const int &err);
int recvdata(int sockfd, char *precvdata);

int main()
{
    /**
     * （1）连接 server 。
    */
    size_t senddatacount = 0;
    size_t sendping = 0;
    int clientfd = 0;
    int r = connectserver(clientfd, SERVERIP, SERVERPORT);
    if (r == 0)
    {
        std::cout << "连接 server 成功！" << std::endl;
    }
    else
    {
        std::cout << "连接 server 失败！" << std::endl;
        return 1;
    }

    /**
     * （2）组合向 server 发送的数据。
    */
    XMNCRC32 &crc32 = SingletonBase<XMNCRC32>::GetInstance();

    size_t pkgheaderlen = sizeof(XMNPkgHeader);
    size_t registerinfolen = sizeof(RegisterInfo);
    char *sendbuf = new char[pkgheaderlen + registerinfolen];

    XMNPkgHeader *ppkgheader = (XMNPkgHeader *)sendbuf;
    ppkgheader->pkglen = htons(pkgheaderlen + registerinfolen);
    ppkgheader->msgcode = htons(CMD_LOGIC_REGISTER);

    RegisterInfo *pregisterinfo = (RegisterInfo *)(sendbuf + pkgheaderlen);
    pregisterinfo->type = 0;
    std::strcpy(pregisterinfo->username, "xuchanglong");
    std::strcpy(pregisterinfo->password, "123456");
    ppkgheader->crc32 = crc32.GetCRC32((unsigned char *)pregisterinfo, registerinfolen);
    ppkgheader->crc32 = htonl(ppkgheader->crc32);

    const size_t kRegisterCRC32 = ppkgheader->crc32;
    const size_t kPingCRC32 = 0;
    char recvbuf[200] = {0};
    XMNPkgHeader *ppkgheader_tmp = nullptr;

    /**
     * （3）循环收发数据。
    */
    while (true)
    {
        std::cout << std::endl;
        /**
         * 注册指令。
        */
        ppkgheader->msgcode = htons(CMD_LOGIC_REGISTER);
        ppkgheader->crc32 = kRegisterCRC32;
        ppkgheader->pkglen = htons(pkgheaderlen + registerinfolen);
        senddata(clientfd, sendbuf, pkgheaderlen + registerinfolen);
        senddatacount++;
        std::cout << "已发送 " << senddatacount << " 个注册包。" << std::endl;

        if (recvdata(clientfd, recvbuf) < 0)
        {
            std::cout << "数据接收失败。" << std::endl;
        }

        ppkgheader_tmp = (XMNPkgHeader *)recvbuf;
        RegisterInfo *pregisterinfo = (RegisterInfo *)(recvbuf + pkgheaderlen);

        std::cout << "len：" << ntohs(ppkgheader->pkglen) << std::endl;
        std::cout << "username：" << pregisterinfo->username << std::endl;
        std::cout << "password：" << pregisterinfo->password << std::endl;

        /**
         * 心跳包。
        */
        ppkgheader->msgcode = htons(CMD_LOGIC_PING);
        ppkgheader->crc32 = kPingCRC32;
        ppkgheader->pkglen = htons(pkgheaderlen);
        senddata(clientfd, sendbuf, pkgheaderlen);
        sendping++;
        std::cout << "已发送 " << sendping << " 个心跳包。" << std::endl;

        if (recvdata(clientfd, recvbuf) < 0)
        {
            std::cout << "数据接收失败。" << std::endl;
        }
        ppkgheader_tmp = (XMNPkgHeader *)recvbuf;
        if (ntohs(ppkgheader->msgcode) == CMD_LOGIC_PING)
        {
            std::cout << "心跳包接收成功。" << std::endl;
        }

        if (sendping > 3)
        {
            break;
        }
        sleep(5);
    }
    close(clientfd);
    return 0;
}

int connectserver(int &clientfd, const std::string &strserverip, const size_t &port)
{
    int r = 0;

    /**
     * （1）创建连接 socket 。
    */
    clientfd = socket(AF_INET, SOCK_STREAM, 0);

    /**
     * （2）设置收发的超时时间。
    */
    struct timeval sendtimeout = {3, 0};
    struct timeval recvtimeout = {3, 0};
    if (setsockopt(clientfd, SOL_SOCKET, SO_SNDTIMEO, (void *)&sendtimeout, sizeof(struct timeval)) != 0)
    {
        showerrorinfo("setsockopt", r, errno);
        return 2;
    }
    if (setsockopt(clientfd, SOL_SOCKET, SO_RCVTIMEO, (void *)&recvtimeout, sizeof(struct timeval)) != 0)
    {
        showerrorinfo("setsockopt", r, errno);
        return 3;
    }

    /**
     * （3）连接 server 。
    */
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    inet_pton(AF_INET, strserverip.c_str(), &serveraddr.sin_addr.s_addr);
    r = connect(clientfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (r == -1)
    {
        showerrorinfo("connect", r, errno);
        return 1;
    }

    return 0;
}

void senddata(int clientfd, char *buf, const size_t &buflen)
{
    size_t uwrote = 0;
    int lentmp = 0;
    while (uwrote < buflen)
    {
        lentmp = send(clientfd, buf + uwrote, buflen - uwrote, 0);
        if (lentmp <= 0)
        {
            showerrorinfo("connect", lentmp, errno);
            return;
        }
        uwrote += lentmp;
    }
    return;
}

void showerrorinfo(const std::string &strfun, const int &ireturnvalue, const int &err)
{
    std::cout << strfun << " error,return value is \"" << ireturnvalue << "\""
              << ",error code is \"" << err << "\""
              << ",error info is \"" << strerror(err) << "\""
              << "." << std::endl;
}

int recvdata(int sockfd, char *precvdata)
{
    if (precvdata == nullptr)
    {
        return -2;
    }
    const size_t kPkgHeaderLen = sizeof(XMNPkgHeader);
    char *pbuftmp = precvdata;
    size_t recvlen = kPkgHeaderLen;

    /**
     * （1）接收包头数据。
    */
    int recvdatacount = recv(sockfd, pbuftmp, recvlen, 0);
    if (recvdatacount < 0)
    {
        return -1;
    }

    if (recvdatacount < recvlen)
    {
        /**
         * 包头没有收全。
        */
    lblrecvpkgheader:
        pbuftmp += recvdatacount;
        recvlen -= recvdatacount;
        /**
         * 继续接收包头。
        */
        recvdatacount = recv(sockfd, pbuftmp, recvlen, 0);
        if (recvdatacount < 0)
        {
            return -1;
        }
        else if (recvdatacount < recvlen)
        {
            goto lblrecvpkgheader;
        }
        goto lblrecvpkgbody1;
    }

    /**
     * （2）开始接收包体数据。
    */
lblrecvpkgbody1:
    XMNPkgHeader *ppkgheader = (XMNPkgHeader *)precvdata;
    const size_t kPkgLen = ntohs(ppkgheader->pkglen);
    if (kPkgLen == kPkgHeaderLen)
    {
        /**
         * 此包数据只包含一个包头。
         * 接收数据完毕！
        */
        return kPkgLen;
    }
    recvlen = kPkgLen - kPkgHeaderLen;
    pbuftmp = precvdata + kPkgHeaderLen;
lblrecvpkgbody2:
    recvdatacount = recv(sockfd, pbuftmp, recvlen, 0);
    if (recvdatacount < 0)
    {
        return -1;
    }
    if (recvdatacount < recvlen)
    {
        pbuftmp += recvdatacount;
        recvlen -= recvdatacount;
        goto lblrecvpkgbody2;
    }

    return kPkgLen;
}