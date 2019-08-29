#include <errno.h>
#include "xmn_socket.h"
#include "xmn_macro.h"
#include "xmn_func.h"

int XMNSocket::WaitRequestHandler(XMNConnSockInfo *pconnsockinfo)
{
    xmn_log_stderr(errno, "The data has arrvied.");
    return 0;
}