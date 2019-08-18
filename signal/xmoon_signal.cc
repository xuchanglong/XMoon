#include <signal.h>
#include <iostream>
#include "xmn_global.h"
#include "xmn_func.h"
#include "xmn_macro.h"
#include <cstring>
#include "sys/wait.h"

struct XMNSignal
{
    /**
          *  信号的编号。 
          */
    int signum;
    /**
          * 信号的名称。
          */
    const char *psigname;
    /**
          *  信号的处理函数。
          */
    void (*phandler)(int signum, siginfo_t *psiginfo, void *content);
};

/**
 * @function   信号处理函数。
 * @paras           signum      信号的编号。
 *                           psiginfo      信号相关信息的结构体。
 *                           pcontext   备用。
 * @return         none 。
 * @author       xuchanglong
 * @time            2019-08-17
 */
static void SignalHandler(int signum, siginfo_t *psiginfo, void *pcontent);

/**
 * @function    获取子进程的状态，避免子进程变成僵尸进程。
 * @paras           none 。
 * @return          none 。
 * @author        xuchanglong
 * @time            2019-08-18
*/
static void XMNProcessGetStatus();

/**
 *  定义本系统处理的各种信号。 
 */
XMNSignal signalinfo[] = {
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
    for (psig = signalinfo; psig->signum; psig++)
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
            xmn_log_info(XMN_LOG_EMERG, errno, "sigaction(%s) failed.", psig->psigname);
            return -1;
        }
        else
        {
            xmn_log_stderr(XMN_LOG_STDERR, "sigaction(%s) succed!", psig->psigname);
        }
    }
    return 0;
}

static void SignalHandler(int signum, siginfo_t *psiginfo, void *pcontent)
{
    XMNSignal *psi = nullptr;
    char *action = (char *)"";
    /**
     * 遍历信号数组。
    */
    for (psi = signalinfo; psi->signum; ++psi)
    {
        if (psi->signum == signum)
        {
            break;
        }
    }
    /**
     * 分别不同的进程对信号进行的处理。
    */
    if (g_xmn_process_type == XMN_PROCESS_MASTER)
    {
        if (signum == SIGCHLD)
        {
            g_xmn_reap = 1;
        }
        /**
         * 这里添加对其他信号的处理。
        */
    }
    else if (g_xmn_process_type == XMN_PROCESS_WORKER)
    {
        /**
         * 这里添加子进程对信号的处理。
        */
    }
    else
    {
        /**
         * 这里添加其他进程对信号的处理。
        */
    }
    /**
     * 对这次信号进行日志记录。
    */
    if (psiginfo && psiginfo->si_pid)
    {
        xmn_log_info(XMN_LOG_NOTICE, errno, "Signal %d (%s) received from %d %s", signum, psi->psigname, psiginfo->si_pid, action);
    }
    else
    {
        xmn_log_info(XMN_LOG_NOTICE, errno, "Signal %d (%s) received from %s", signum, psi->psigname, action);
    }
    /**
     * 对信号进行处理。
    */
    if (signum == SIGCHLD)
    {
        XMNProcessGetStatus();
    }
}

void XMNProcessGetStatus()
{
    /**
     * 用于保存子进程的终止状态。
    */
    int status;
    /**
     * 是否已经有子进程退出。
    */
    int one = 0;
    /**
     * 终止进程的pid。
    */
    pid_t pid;
    /**
     * 保存调用函数出错的代码。
    */
    int err;

    while (true)
    {
        pid = waitpid(-1, &status, WNOHANG);
        /**
         * 没有子进程退出。
        */
        if (pid == 0)
        {
            return;
        }
        /**
         * 调用出错。
        */
        else if (pid == -1)
        {
            err = errno;
            /**
             * 被中断打断，重新来一次。
            */
            if (err == EINTR)
            {
                continue;
            }
            /**
             * 该父进程已经不存在子进程了。
            */
            else if (err == ECHILD && one)
            {
                return;
            }
            else if (err == ECHILD)
            {
                xmn_log_info(XMN_LOG_INFO, err, "waitpid() failed.");
                return;
            }
        }
        one = 1;
        if (WTERMSIG(status))
        {
            /**
             * 获取使子进程终止的信号编号。
            */
            xmn_log_info(XMN_LOG_NOTICE, 0, "pid = %d exited on signal %d.", pid, WTERMSIG(status));
        }
        else
        {
            /**
             * WEXITSTATUS()获取子进程传递给exit或者_exit参数的低八位。
            */
            xmn_log_info(XMN_LOG_NOTICE, 0, "pid = %d exited on code %d.", pid, WTERMSIG(status));
        }
    }
}
