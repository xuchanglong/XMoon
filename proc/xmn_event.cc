#include "xmn_global.h"

int XMNProcessEventsTimers()
{
    /**
     * （1）处理网络事件。
    */
    g_socket.EpollProcessEvents(-1);

    /**
     * （2）在终端显示统计信息。
    */
    g_socket.PrintInfo();
    
    return 0;
}