#include "comm/xmn_socket_logic.h"
#include "comm/xmn_socket_logic_comm.h"
#include "xmn_memory.h"
#include "xmn_func.h"
#include "xmn_crc32.h"
#include "xmn_lockmutex.hpp"

#include "netinet/in.h"
#include <string.h>

/**
 * TODO：此处为什么函数指针必须是 XMNSocketLogic 作用域下的。
 * 定义一个函数指针类型，该函数指针仅仅指向符合要求的 XMNSocketLogic 的成员函数。
*/
using MsgHandler = int (XMNSocketLogic::*)(XMNMsgHeader *pmsgheader, char *ppkgbody, size_t pkgbodylen);

/**
 * 保存各种业务处理函数的数据。
*/
static const MsgHandler msghandlerall[] =
    {
        &XMNSocketLogic::HandlePing,     //【0】
        nullptr,                         //【1】
        nullptr,                         //【2】
        nullptr,                         //【3】
        nullptr,                         //【4】
        &XMNSocketLogic::HandleRegister, //【5】
        &XMNSocketLogic::HandleLogin,    //【6】
};

#define TOTAL_COMMANDS (sizeof(msghandlerall) / sizeof(MsgHandler))

XMNSocketLogic::XMNSocketLogic()
{
    ;
}

XMNSocketLogic::~XMNSocketLogic()
{
    ;
}

int XMNSocketLogic::Initialize()
{
    return XMNSocket::Initialize();
}

int XMNSocketLogic::HandleRegister(XMNMsgHeader *pmsgheader, char *ppkgbody, size_t pkgbodylen)
{
    /**
     * （1）判断数据包的合法性。
     */
    if ((pmsgheader == nullptr) || (ppkgbody == nullptr))
    {
        return -1;
    }

    if (pkgbodylen != sizeof(RegisterInfo))
    {
        return -2;
    }
    XMNConnSockInfo *pconnsockinfo = pmsgheader->pconnsockinfo;
    /*
     * （2）对该业务逻辑处理进行加锁。
     * 解释：对于同一个用户，可能同时发送来多个请求过来，造成多个线程同时为该 用户服务。
     * 比如以网游为例，用户要在商店中买A物品，又买B物品，而用户的钱 只够买A或者B，不够同时买A和B呢？
     * 那如果用户发送购买命令过来买了一次A，又买了一次B，如果是两个线程来执行同一个用户的这两次不同的购买命令，
     * 很可能造成这个用户购买成功了 A，又购买成功了 B。
     * 所以根据上述考虑，同一个连接多个逻辑进行加锁处理。
    */
    XMNLockMutex lockmutex_logic(&pconnsockinfo->logicprocmutex);
    /**
     * （3）获取发送来的所有数据。
    */
    RegisterInfo *pregisterinfo = (RegisterInfo *)ppkgbody;
    pregisterinfo->type = ntohs(pregisterinfo->type);
    /**
     * 防止该包是畸形包，如果后面没有'\0'，容易造成字符串溢出漏洞。
    */
    pregisterinfo->username[sizeof(pregisterinfo->username) - 1] = 0;
    pregisterinfo->password[sizeof(pregisterinfo->password) - 1] = 0;
    /**
     * （4）各种业务处理。
    */

    /**
     * ------------------------------------------------------------------
    */

    /**
     * （5）组合回复的数据。
    */
    XMNMemory &memory = SingletonBase<XMNMemory>::GetInstance();
    XMNCRC32 &crc32 = SingletonBase<XMNCRC32>::GetInstance();
    char *psenddata = (char *)memory.AllocMemory(kMsgHeaderLen_ + kPkgHeaderLen_ + sizeof(RegisterInfo), false);
    // a、消息头。
    XMNMsgHeader *pmsgheader_send = (XMNMsgHeader *)psenddata;
    memcpy(pmsgheader_send, pmsgheader, sizeof(XMNMsgHeader));
    //b、包头
    XMNPkgHeader *ppkgheader_send = (XMNPkgHeader *)(psenddata + kMsgHeaderLen_);
    ppkgheader_send->pkglen = htons(kPkgHeaderLen_ + sizeof(RegisterInfo));
    ppkgheader_send->msgcode = htons(CMD_LOGIC_REGISTER);
    //c、包体
    char *ppkgbody_send = psenddata + kMsgHeaderLen_ + kPkgHeaderLen_;
    memcpy(ppkgbody_send, pregisterinfo, sizeof(RegisterInfo));
    ppkgheader_send->crc32 = htonl(crc32.GetCRC32((unsigned char *)ppkgbody_send, sizeof(RegisterInfo)));

    /**
     * （6）将可写标志加入 epoll 红黑树中。
    */
    /*
    if (EpollOperationEvent(pconnsockinfo->fd, EPOLL_CTL_MOD, EPOLLOUT, 0, pconnsockinfo) != 0)
    {
        xmn_log_stderr(0,"XMNSocketLogic::HandleRegister() 中 EpollOperationEvent 执行失败。");
    }
    */

    /**
     * （7）将待发送的数据压入发送队列中。
    */
    PutInSendDataQueue(psenddata);

    //xmn_log_stderr(0, "执行了 XMNSocketLogic::HandleRegister 函数。");
    return 0;
}

int XMNSocketLogic::HandleLogin(XMNMsgHeader *pmsgheader, char *ppkgbody, size_t pkgbodylen)
{
    xmn_log_stderr(0, "执行了 XMNSocketLogic::HandleLogin 函数。");
    return 0;
}

void XMNSocketLogic::ThreadRecvProcFunc(char *pmsgbuf)
{
    /**
     * （1）变量声明。
    */
    XMNMsgHeader *pmsgheader = (XMNMsgHeader *)pmsgbuf;
    XMNPkgHeader *ppkgheader = (XMNPkgHeader *)(pmsgbuf + kMsgHeaderLen_);
    char *ppkgbody = nullptr;
    unsigned short msgcode = 0;
    XMNConnSockInfo *pconnsockinfo = nullptr;
    /**
     * 拷贝连接块信息，防止在处理该消息时，该连接已经断开并且该连接块被其他新的连接所使用。
    */
    //XMNConnSockInfo *pconnsockinfo = new XMNConnSockInfo;
    //memcpy(pconnsockinfo, pmsgheader->pconnsockinfo, sizeof(XMNConnSockInfo) * 1);

    size_t pkglen = ntohs(ppkgheader->pkglen);
    size_t pkgbodylen = pkglen - kPkgHeaderLen_;

    /**
     * （2）CRC32 校验。
    */
    if (pkglen == kPkgHeaderLen_)
    {
        /**
         * 没有包体的校验码应该为 0 。
        */
        if (ppkgheader->crc32 != 0)
        {
            goto lblexit;
        }
        ppkgbody = nullptr;
    }
    else
    {
        ppkgbody = pmsgbuf + kMsgHeaderLen_ + kPkgHeaderLen_;
        int crc32src = ntohl(ppkgheader->crc32);
        int crc32 = SingletonBase<XMNCRC32>::GetInstance().GetCRC32((unsigned char *)ppkgbody, pkgbodylen);
        if (crc32 != crc32src)
        {
            xmn_log_stderr(0, "XMNSocketLogic::ThreadRecvProcFunc 中 crc32 校验失败，丢弃数据。");
            goto lblexit;
        }
    }

    /**
     * （3）废弃包处理。
    */
    /**
     * 如果 client 发来了数据包，server 激活线程池中一个线程去处理，在这个过程中 client 与 server 断开了连接，
     * 那么该消息就不必处理。
    */
    pconnsockinfo = pmsgheader->pconnsockinfo;
    if (pconnsockinfo->currsequence != pmsgheader->currsequence)
    {
        goto lblexit;
    }
    /**
     * 判断消息码的范围。
    */
    msgcode = ntohs(ppkgheader->msgcode);
    if (msgcode >= TOTAL_COMMANDS)
    {
        xmn_log_stderr(0, "XMNSocketLogic::ThreadRecvProcFunc()中的 msgCode = %d 消息码不对!", msgcode);
        goto lblexit;
    }

    /**
     * （4）调用相关的消息处理函数处理。
     * 能够执行到这里，说明该数据包是完整的，没有过期。
    */
    if (msghandlerall[msgcode] == nullptr)
    {
        xmn_log_stderr(0, "XMNSocketLogic::ThreadRecvProcFunc()中的 msgcode = %d 消息码找不到对应的处理函数。", msgcode);
        goto lblexit;
    }
    (this->*msghandlerall[msgcode])(pmsgheader, (char *)ppkgbody, pkgbodylen);

lblexit:
    //delete pconnsockinfo;
    //pconnsockinfo = nullptr;
    return;
}

int XMNSocketLogic::HandlePing(XMNMsgHeader *pmsgheader, char *ppkgbody, size_t pkgbodylen)
{
    if (pmsgheader == nullptr)
    {
        return -1;
    }
    if (pkgbodylen != 0)
    {
        return -2;
    }

    XMNConnSockInfo *pconnsockinfo = pmsgheader->pconnsockinfo;
    XMNLockMutex lockmutex_logic(&pconnsockinfo->logicprocmutex);
    pconnsockinfo->lastpingtime = time(nullptr);

    SendNoBodyData2Client(pmsgheader, CMD_LOGIC_PING);

    //xmn_log_stderr(0, "成功地收到了心跳包并发送。");
    return 0;
}

void XMNSocketLogic::SendNoBodyData2Client(XMNMsgHeader *pmsgheader, const uint16_t &kMsgCode)
{
    XMNMemory &memory = SingletonBase<XMNMemory>::GetInstance();
    char *psenddata = (char *)memory.AllocMemory(kMsgHeaderLen_ + kPkgHeaderLen_, false);

    memcpy(psenddata, pmsgheader, sizeof(char) * kMsgHeaderLen_);
    XMNPkgHeader *ppkgheader = (XMNPkgHeader *)(psenddata + kMsgHeaderLen_);

    ppkgheader->pkglen = htons(kPkgHeaderLen_);
    ppkgheader->msgcode = htons(kMsgCode);
    ppkgheader->crc32 = 0;

    PutInSendDataQueue(psenddata);

    return;
}

int XMNSocketLogic::PingTimeOutChecking(XMNMsgHeader *pmsgheader, time_t currenttime)
{
    if (pmsgheader == nullptr)
    {
        return -1;
    }
    XMNMemory &memory = SingletonBase<XMNMemory>::GetInstance();
    XMNConnSockInfo *pconnsockinfo = pmsgheader->pconnsockinfo;
    if (pmsgheader->currsequence == pconnsockinfo->currsequence)
    {
        /**
         * 此连接没有断开。
        */
        if ((currenttime - pconnsockinfo->lastpingtime) > (pingwaittime_ * 3))
        {
            xmn_log_stderr(0, "超时不发心跳包，连接被关闭。");
            ActivelyCloseSocket(pconnsockinfo);
        }
        memory.FreeMemory(pmsgheader);
    }
    else
    {
        /**
         * 此连接已经断开。
        */
        memory.FreeMemory(pmsgheader);
    }
}