#ifndef XMOON__INCLUDE_XMN_COMM_H_
#define XMOON__INCLUDE_XMN_COMM_H_

/****************************************************
 * 定义 server 接收 client 发来的数据的状态。
****************************************************/
enum RecvStatus
{
    /**
    * 开始接收数据的包头。
    */
    PKG_HD_INIT = 0,
    /**
    * 包头没有接收完，需要继续接收。
    */
    PKG_HD_RECVING = 1,
    /**
    * 包头接收完毕，开始接收包体。
    */
    PKG_BD_INIT = 2,
    /**
    * 包体没有接收完，需要继续继续接收包体。
    */
    PKG_BD_RECVING = 3
};

/**
 * XMNListenSockInfo 中存放包头的数组的大小。
*/
#define XMN_PKG_HEADER_SIZE 20

/**
 * 包头结构。
*/
struct XMNPkgHeader
{
    /**
     * 报文的总长度（包头 + 包体）。
    */
    unsigned short pkglen;

    /**
     * 消息类型的代码，用于区别不同的命令（消息）。
    */
    unsigned short msgCode;

    /**
     * CRC32 校验，用于防止接收到的数据和 client 发送的数据不符的问题。
    */
    int crc32;
};

#endif