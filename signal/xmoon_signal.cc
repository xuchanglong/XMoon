#include <signal.h>
#include <iostream>
#include "xmn_func.h"
#include "xmn_macro.h"
#include <cstring>

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
    {0, nullptr, nullptr}};

/**
     *  信号的初始化函数。 
     */
int XMNSignalInit()
{
    /**
     * 变量初始化。
     */
    XMNSignal *psig = nullptr;
    /*
    struct sigaction 
    {
        void (*sa_handler)(int);
        void (*sa_sigaction)(int, siginfo_t *, void *);
        sigset_t sa_mask;
        int sa_flags;
        void (*sa_restorer)(void);
    }
    */
    struct sigaction sa;

    /**
     * 设置每一个信号的信号处理程序。
     */
    for (psig = SignalInfo; psig->signum; psig++)
    {
        /**
         * 设置信号的处理函数。
        */
        memset(&sa, 0, sizeof(struct sigaction) * 1);
        if (psig->phandler)
        {
            sa.sa_sigaction = psig->phandler;
            sa.sa_flags = SA_SIGINFO;
        }
        else
        {
            /**
             * 忽略信号的处理程序，否则OS会执行默认的处理程序，很有可能是杀死系统。
            */
            sa.sa_handler = SIG_IGN;
        }
        /**
         * 接收信号时不堵塞其他信号。
        */
        sigemptyset(&sa.sa_mask);

        if (sigaction(psig->signum, &sa, nullptr) == -1)
        {
            xmn_log_error_core(XMN_LOG_EMERG, errno, "sigaction(%s) failed.", psig->strsigname);
            return -1;
        }
        else
        {
            xmn_log_stderr(XMN_LOG_STDERR, "sigaction(%s) succed!", psig->strsigname);
        }
    }
    return 0;
}

static void SignalHandler(int signum, siginfo_t *psiginfo, void *pcontent)
{
    std::cout << "The signal " << signum << " is coming." << std::endl;
}
