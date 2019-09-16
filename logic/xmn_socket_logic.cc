#include "xmn_socket_logic.h"
#include "xmn_func.h"

/**
 * TODO：此处为什么函数指针必须是 XMNSocketLogic 作用域下的。
*/
using pmsghandler = int (XMNSocketLogic::*)(XMNConnSockInfo *pconnsockinfo,
                                            XMNMsgHeader *pmsgheader,
                                            char *ppkgbody,
                                            size_t pkgbodylen);

static const pmsghandler statushandler[] =
    {
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        &XMNSocketLogic::HandleRegister,
        &XMNSocketLogic::HandleLogin};

#define TOTAL_COMMANDS (sizeof(statushandler) / sizeof(pmsghandler))

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
    ;
}