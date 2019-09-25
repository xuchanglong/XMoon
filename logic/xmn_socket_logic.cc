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
using pmsghandler = int (XMNSocketLogic::*)(XMNConnSockInfo *pconnsockinfo,
                                            XMNMsgHeader *pmsgheader,
                                            char *ppkgbody,
                                            size_t pkgbodylen);

/**
 * 保存各种业务处理函数的数据。
*/
static const pmsghandler msghandlerall[] =
    {
        nullptr,                         //【0】
        nullptr,                         //【1】
        nullptr,                         //【2】
        nullptr,                         //【3】
        nullptr,                         //【4】
        &XMNSocketLogic::HandleRegister, //【5】
        &XMNSocketLogic::HandleLogin,    //【6】
};

#define TOTAL_COMMANDS (sizeof(msghandlerall) / sizeof(pmsghandler))

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

int XMNSocketLogic::HandleRegister(
    XMNConnSockInfo *pconnsockinfo,
    XMNMsgHeader *pmsgheader,
    char *ppkgbody,
    size_t pkgbodylen)
{
    /**
     * （1）判断数据包的合法性。
     */
    if ((pconnsockinfo == nullptr) || (pmsgheader == nullptr) || (ppkgbody == nullptr))
    {
        return -1;
    }

    if (pkgbodylen != sizeof(RegisterInfo))
    {
        return -2;
    }

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
    /**
     * （4）各种业务处理。
    */

    /**
     * ------------------------------------------------------------------
    */

    /**
     * （5）组合回复的数据。
    */
    XMNMemory *pmemory = SingletonBase<XMNMemory>::GetInstance();
    XMNCRC32 *pcrc32 = SingletonBase<XMNCRC32>::GetInstance();
    char *psenddata = (char *)pmemory->AllocMemory(msgheaderlen_ + pkgheaderlen_ + sizeof(RegisterInfo), false);
    // a、消息头。
    XMNMsgHeader *pmsgheader_send = (XMNMsgHeader *)psenddata;
    memcpy(pmsgheader_send, pmsgheader, sizeof(XMNMsgHeader));
    //b、包头
    XMNPkgHeader *ppkgheader_send = (XMNPkgHeader *)(psenddata + msgheaderlen_);
    ppkgheader_send->pkglen = htons(pkgheaderlen_ + sizeof(RegisterInfo));
    ppkgheader_send->msgcode = htons(CMD_LOGIC_REGISTER);
    //c、包体
    char *ppkgbody_send = psenddata + msgheaderlen_ + pkgheaderlen_;
    memcpy(ppkgbody_send, pregisterinfo, sizeof(RegisterInfo));
    ppkgheader_send->crc32 = htonl(pcrc32->GetCRC((unsigned char *)ppkgbody_send, sizeof(RegisterInfo)));

    /**
     * （6）将可写标志加入 epoll 红黑树中。
    */
    if (EpollOperationEvent(pconnsockinfo->fd, EPOLL_CTL_MOD, EPOLLOUT, 0, pconnsockinfo) != 0)
    {
        xmn_log_stderr(0,"XMNSocketLogic::HandleRegister() 中 EpollOperationEvent 执行失败。");
    }

    xmn_log_stderr(0, "执行了 XMNSocketLogic::HandleRegister 函数。");
    return 0;
}

int XMNSocketLogic::HandleLogin(
    XMNConnSockInfo *pconnsockinfo,
    XMNMsgHeader *pmsgheader,
    char *ppkgbody,
    size_t pkgbodylen)
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
    XMNPkgHeader *ppkgheader = (XMNPkgHeader *)(pmsgbuf + msgheaderlen_);
    char *ppkgbody = nullptr;
    unsigned short msgcode = 0;
    XMNConnSockInfo *pconnsockinfo = nullptr;
    /**
     * 拷贝连接块信息，防止在处理该消息时，该连接已经断开并且该连接块被其他新的连接所使用。
    */
    //XMNConnSockInfo *pconnsockinfo = new XMNConnSockInfo;
    //memcpy(pconnsockinfo, pmsgheader->pconnsockinfo, sizeof(XMNConnSockInfo) * 1);

    size_t pkglen = ntohs(ppkgheader->pkglen);
    size_t pkgbodylen = pkglen - pkgheaderlen_;

    /**
     * （2）CRC32 校验。
    */
    if (pkglen == pkgheaderlen_)
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
        ppkgbody = pmsgbuf + msgheaderlen_ + pkgheaderlen_;
        int crc32src = ntohl(ppkgheader->crc32);
        int crc32 = SingletonBase<XMNCRC32>::GetInstance()->GetCRC((unsigned char *)ppkgbody, pkgbodylen);
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
    (this->*msghandlerall[msgcode])(pconnsockinfo, pmsgheader, (char *)ppkgbody, pkgbodylen);

lblexit:
    //delete pconnsockinfo;
    //pconnsockinfo = nullptr;
    return;
}