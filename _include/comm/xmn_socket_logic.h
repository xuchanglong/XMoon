#ifndef XMOON__INCLUDE_XMNSOCKETLOGIC_H_
#define XMOON__INCLUDE_XMNSOCKETLOGIC_H_

#include "comm/xmn_socket.h"

class XMNSocketLogic : public XMNSocket
{
public:
    XMNSocketLogic();
    virtual ~XMNSocketLogic();

public:
    virtual int Initialize();

    int HandleRegister(
        XMNConnSockInfo *pconnsockinfo,
        XMNMsgHeader *pmsgheader,
        char *ppkgbody,
        size_t pkgbodylen);

    int HandleLogin(
        XMNConnSockInfo *pconnsockinfo,
        XMNMsgHeader *pmsgheader,
        char *ppkgbody,
        size_t pkgbodylen);

public:
    virtual void ThreadRecvProcFunc(char *pmsgbuf);
};

#endif