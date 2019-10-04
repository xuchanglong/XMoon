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

public:
    int HandleRegister(
        XMNMsgHeader *pmsgheader,
        char *ppkgbody,
        size_t pkgbodylen);

    int HandleLogin(
        XMNMsgHeader *pmsgheader,
        char *ppkgbody,
        size_t pkgbodylen);

    int HandlePing(
        XMNMsgHeader *pmsgheader,
        char *ppkgbody,
        size_t pkgbodylen);

    /**
     * @function 向 client 发送无包体的数据。
     * @paras   pmsgheader  消息头。
     *          kMsgCode    指令。
     * @return  none 。
     * @author  xuchanglong
     * @time    2019-10-04
    */
    void SendNoBodyData2Client(XMNMsgHeader *pmsgheader, const uint16_t &kMsgCode);

public:
    virtual void ThreadRecvProcFunc(char *pmsgbuf);
    virtual int PingTimeOutChecking(XMNMsgHeader *pmsgheader, time_t currenttime);
};

#endif