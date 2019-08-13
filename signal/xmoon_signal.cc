#include <signal.h>
#include <iostream>

struct XMNSignal
{
    /**
          *  信号的编号。 
          */
    int signum;
    /**
          * 信号的名称。
          */
    std::string strsigname;
    /**
          *  信号的处理函数。
          */
    void (*phandler)(int signum, siginfo_t *psiginfo, void *content);
};

/**
 *  信号处理函数。  
 */
static void SignalHandler(int signum, siginfo_t *psiginfo, void *pcontent);

/**
 *  定义本系统处理的各种信号。 
 */
XMNSignal SignalInfo[] = {
    /**
     *  终端断开信号。常用于向守护进程发送 reload 配置文件的通知。 
     */
    {SIGHUP, "SIGHUP", SignalHandler},
    /**
     *  ctrl + c 信号。 
     */
    {SIGINT, "SIGINT", SignalHandler},
    /**
     *  kill(1)，系统默认的终止信号。 
     */
    {SIGTERM, "SIGTERM", SignalHandler},
    /**
     *  子进程终止信号。  
     */
    {SIGCHLD, "SIGCHLD", SignalHandler},
    /**
     *  ctrl + '\' 
     */
    {SIGQUIT, "SIGQUIT", SignalHandler},
    /**
     *  异步 IO 事件。
     */
    {SIGIO, "SIGIO", SignalHandler},
    /**
     *  无效的系统调用信号。选择忽略。 
     */
    {SIGSYS, "SIGSYS,SIG_IGN", SignalHandler},
    /**
     *  信号 > 0，故用 0 表示末尾。 
     */
    {0, nullptr, nullptr}
};

/**
     *  信号的初始化函数。 
     */
int XMNSignalInit()
{
    ; 
}

static void SignalHandler(int signum, siginfo_t *psiginfo, void *pcontent)
{
    std::cout << "The signal " << signum << " is coming." << std::endl;
}
