#include "xmn_socket_logic.h"
#include "xmn_func.h"
#include "xmn_crc32.h"
#include "netinet/in.h"

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
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        &XMNSocketLogic::HandleRegister,
        &XMNSocketLogic::HandleLogin};

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
    XMNConnSockInfo *pconnsockinfo = (XMNConnSockInfo *)pmsgheader->pconnsockinfo;
    size_t pkglen = ntohs(ppkgheader->pkglen);
    size_t pkgbodylen = pkglen - pkgheaderlen_;

    /**
     * （2）CRC32 校验。
    */
    /**
     * 没有包体的校验码应该为 0 。
    */
    if (pkglen == pkgheaderlen_)
    {
        if (ppkgheader->crc32 != 0)
        {
            return;
        }
        ppkgbody = nullptr;
    }
    else
    {
        ppkgbody = pmsgbuf + msgheaderlen_ + pkgheaderlen_;
        ppkgheader->crc32 = ntohl(ppkgheader->crc32);
        int crc32 = SingletonBase<XMNCRC32>::GetInstance()->GetCRC((unsigned char *)ppkgbody, pkgbodylen);
        if (crc32 != ppkgheader->crc32)
        {
            xmn_log_stderr(0, "XMNSocketLogic::ThreadRecvProcFunc 中 crc32 校验失败，丢弃数据。");
            return;
        }
    }

    /**
     * （3）废弃包处理。
    */
    /**
     * 如果 client 发来了数据包，server 激活线程池中一个线程去处理，在这个过程中 client 与 server 断开了连接，
     * 那么该消息就不必处理。
    */
    if (pconnsockinfo->currsequence != pmsgheader->currsequence)
    {
        return;
    }
    /**
     * 判断消息码的范围。
    */
    ppkgheader->msgcode = ntohs(ppkgheader->msgcode);
    unsigned short msgcode = ppkgheader->msgcode;
    if (msgcode >= TOTAL_COMMANDS)
    {
        xmn_log_stderr(0, "XMNSocketLogic::ThreadRecvProcFunc()中的msgCode=%d消息码不对!", msgcode);
        return;
    }

    /**
     * （4）调用相关的消息处理函数处理。
     * 能够执行到这里，说明该数据包是完整的，没有过期。
    */
    if (msghandlerall[msgcode] == nullptr)
    {
        xmn_log_stderr(0, "XMNSocketLogic::ThreadRecvProcFunc()中的 msgcode = %d 消息码找不到对应的处理函数。", msgcode);
        return;
    }
    (this->*msghandlerall[msgcode])(pconnsockinfo, pmsgheader, (char *)ppkgbody, pkgbodylen);

    return;
}