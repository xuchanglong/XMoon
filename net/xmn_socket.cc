#include "xmn_socket.h"
#include "xmn_config.h"

XMNSocket::XMNSocket()
{
    listenport_count_ = 0;
}

XMNSocket::~XMNSocket()
{
    std::vector<XMNListenPortSockInfo *>::iterator it;
    for (it = vlistenportsockinfolist_.begin(); it != vlistenportsockinfolist_.end(); ++it)
    {
        delete *it;
        *it = nullptr;
    }
    vlistenportsockinfolist_.clear();
}

int XMNSocket::Initialize()
{
    XMNConfig *pconfig = XMNConfig::GetInstance();
    listenport_count_ = atoi(pconfig->GetConfigItem("ListenPortCount").c_str());
    if (listenport_count_ <= 0)
    {
        return -1;
    }
    return OpenListeningSocket(listenport_count_);
}

int XMNSocket::OpenListeningSocket(const size_t &listenportsum)
{
    return 0;
}

int XMNSocket::CloseListeningSocket()
{
    return 0;
}

int XMNSocket::SetNonBlocking(const int &sockfd)
{
    return 0;
}