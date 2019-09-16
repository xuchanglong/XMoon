#ifndef XMOON__INCLUDE_XMN_SLOGIC_H_
#define XMOON__INCLUDE_XMN_SLOGIC_H_

#include "xmn_socket.h"

class XMNLogicSocket : public XMNSocket
{
public:
    XMNLogicSocket();
    virtual ~XMNLogicSocket();

public:
    virtual int Initialize();
    int HandleRegister(XMNConnSockInfo *pconnsockinfo, XMNMsgHeader *pmsgheader, char *ppkgbody, size_t pkgbodylen);
    int HandleLogin(XMNConnSockInfo *pconnsockinfo, XMNMsgHeader *pmsgheader, char *ppkgbody, size_t pkgbodylen);
public:
    virtual void ThreadRecvProcFunc(char *pmsgbuf);
};
#endif