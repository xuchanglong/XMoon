/*****************************************************************************************
 * @function   信号相关操作函数。
 * @time    2019-08-15
*****************************************************************************************/
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
    std::string strsigname;

    /**
     *  信号的处理函数。
    */
    void (*phandler)(int signum, siginfo_t *psiginfo, void *content);
};

/**
 * @function   信号处理函数。
 * @paras   signum      信号的编号。
 *          psiginfo    信号相关信息的结构体。
 *          pcontext    备用。
 * @ret  none 。
 * @time    2019-08-17
 */
static void SignalHandler(int signum, siginfo_t *psiginfo, void *pcontent);

/**
 * @function    获取子进程的状态，避免子进程变成僵尸进程。
 * @paras   none 。
 * @ret  none 。
 * @time    2019-08-18
*/
static void XMNProcessGetStatus();

/**
 *  定义本系统处理的各种信号。 
 */
XMNSignal g_signalinfo[] = {
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
    {0, "", nullptr}};

int XMNSignalInit()
{
    /**
     * （1）定义变量。
     */
    XMNSignal *psig = nullptr;

    /*
    struct sigaction 
    {
        void (*sa_handler)(int);                            //两种信号处理函数。
        void (*sa_sigaction)(int, siginfo_t *, void *);
        sigset_t sa_mask;                                   //指定在信号处理程序中哪些信号会被堵塞。
        int sa_flags;
        void (*sa_restorer)(void);                          //已过时，POSIX 已不再支持。
    }
    */
    // APUE 第3版，P278
    struct sigaction sa;

    /**
     * （2）为每一个信号设置信号处理函数。
     */
    for (psig = g_signalinfo; psig->signum; psig++)
    {
        memset(&sa, 0, sizeof(struct sigaction) * 1);
        /**
         * a、设置信号的处理函数。
        */
        if (psig->phandler)
        {
            sa.sa_sigaction = psig->phandler;
            /**
             * 设置了该标志位，表明信号附带的参数可以传入信号处理函数(sa.sa_sigaction)中。
             * 否则，信号处理函数中访问这些参数会造成段错误。
            */
            sa.sa_flags = SA_SIGINFO;
        }
        else
        {
            /**
             * 忽略信号的处理程序，否则 OS 会执行默认的处理程序，很有可能是杀死系统。
            */
            sa.sa_handler = SIG_IGN;
        }

        /**
         * b、清空被堵塞的信号的集合。
         * sa.sa_mask   指定在执行信号处理函数时哪些信号应该堵塞（屏蔽）。
        */
        sigemptyset(&sa.sa_mask);

        /**
         * c、执行信号安装函数。
        */
        if (sigaction(psig->signum, &sa, nullptr) < 0)
        {
            XMNLogInfo(XMN_LOG_EMERG, errno, "sigaction(%s) failed.", psig->strsigname);
            return -1;
        }
        else
        {
            //XMNLogStdErr(XMN_LOG_STDERR, "sigaction(%s) succesed!", psig->strsigname);
        }
    }
    return 0;
}

static void SignalHandler(int signum, siginfo_t *psiginfo, void *pcontent)
{
    XMNSignal *psi = nullptr;
    char *action = (char *)"";

    /**
     * （1）遍历信号数组。
    */
    bool issignalexit = false;
    for (psi = g_signalinfo; psi->signum; ++psi)
    {
        if (psi->signum == signum)
        {
            issignalexit = true;
            break;
        }
    }
    if (!issignalexit)
    {
        return;
    }

    /**
     * （2）分别对不同的进程的信号进行的处理。
    */
    /**
     * a、对 master 进程收到信号的处理。
    */
    if (g_xmn_process_type == XMN_PROCESS_MASTER)
    {
        /**
         * 子进程终止或者停止。
        */
        if (signum == SIGCHLD)
        {
            g_xmn_reap = 1;
        }

        /**
         * 这里添加对其他信号的处理。
        */
    }
    /**
     * b、对 woker 进程收到信号的处理。
    */
    else if (g_xmn_process_type == XMN_PROCESS_WORKER)
    {
        /**
         * 这里添加 woker 进程对信号的处理。
        */
    }
    /**
     * c、对其他进程收到信号的处理。
    */
    else
    {
        /**
         * 这里添加其他进程对信号的处理。
        */
    }

    /**
     * （3）对这次信号进行日志记录。
     * 信号编号、名称以及发送该信号的进程的 pid 。
    */
    if (psiginfo && psiginfo->si_pid)
    {
        XMNLogInfo(XMN_LOG_NOTICE, errno, "Signal %d (%s) received from %d %s", signum, psi->strsigname, psiginfo->si_pid, action);
    }
    else
    {
        XMNLogInfo(XMN_LOG_NOTICE, errno, "Signal %d (%s) received from %s", signum, psi->strsigname, action);
    }

    /**
     * （4）对信号进行处理。
    */
    if (signum == SIGCHLD)
    {
        XMNProcessGetStatus();
    }
    else
    {
        /* code */
    }
}

void XMNProcessGetStatus()
{
    /**
     * 用于保存子进程的终止状态。
    */
    int statloc;
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
    /**
     * while 循环的意义是将所有已经终结的子进程都处理完毕。
    */
    while (true)
    {
        /**
         * APUE 第3版，P190
         * 等待任何子进程退出，并将子进程终止状态存入 status 。
         * WNOHANG 无子进程终止时不堵塞，直接返回。
        */
        pid = waitpid(-1, &statloc, WNOHANG);

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
             * 该子进程已经不存在。
             * 若子进程退出过，则不是错误，直接返回即可。
            */
            else if (err == ECHILD && one)
            {
                return;
            }
            else if (err == ECHILD)
            {
                XMNLogInfo(XMN_LOG_INFO, err, "waitpid() failed.");
                return;
            }
        }
        one = 1;
        /**
         * 取得子进程因信号而中止的信号。
        */
        if (int sig = WTERMSIG(statloc))
        {
            /**
             * 获取使子进程终止的信号编号。
            */
            XMNLogInfo(XMN_LOG_NOTICE, 0, "pid = %d exited on signal %d.", pid, sig);
        }
        else
        {
            /**
             * 运行到这里说明子进程是正常退出的。
             * WEXITSTATUS()获取子进程传递给 exit 或者 _exit 参数的低八位。
            */
            XMNLogInfo(XMN_LOG_NOTICE, 0, "pid = %d exited on code %d.", pid, WEXITSTATUS(statloc));
        }
    }
}
