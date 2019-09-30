/*****************************************************************************************
 * @function   master 和 wokers 进程工作函数。
 * @author      xuchanglong
 * @time            2019-08-15
*****************************************************************************************/
#include "xmn_macro.h"
#include "xmn_func.h"
#include "xmn_global.h"
#include "xmn_config.h"

#include <signal.h>
#include <iostream>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>

/**
 * master 进程标题。
*/
static std::string g_strmasterprocessname = "master process";

/**
 * @function    开始处理 workers 进程。
 * @paras   workprocesssum  创建 worker 进程的数量。
 * @return  none 。
 * @author  xuchanglong
 * @time    2019-08-15
*/
static void XMNStartWorkerProcess(const size_t &kWorkerProcessCount);

/**
 * @function    创建指定数量的 worker 进程。
 * @paras   inum    子进程编号。
 *          strprocname   子进程的名称。
 * @return  创建的子进程的 pid。
 * @author  xuchanglong
 * @time    2019-08-15
*/
static int XMNCreateProcess(const size_t &kNum, const std::string &kstrProcName);

/**
 * @function    设置子进程的标题以及进入子进程的循环。
 * @paras   inum    子进程编号。
 *          strprocname   子进程的名称。
 * @return  none 。
 * @author  xuchanglong
 * @time    2019-08-15
*/
static int XMNWorkerProcessCycle(const size_t &kNum, const std::string &kstrProcName);

/**
 * @function    子进程开始进入循环之前进行初始化。
 * @paras   inum    子进程编号。
 *          kstrTitleName   进程标题名称。
 * @return  0   操作成功。
 * @author  xuchanglong
 * @time    2019-08-15
*/
static int XMNWorkerProcessInit(const size_t &kNum, const std::string &kstrProcName);

void XMNMasterProcessCycle()
{
    /**
     * （1）添加进程的信号屏蔽掩码，即：当前堵塞而不能递送给该进程的信号集。
    */
    sigset_t set;
    sigemptyset(&set);

    sigaddset(&set, SIGCHLD);
    sigaddset(&set, SIGALRM);
    sigaddset(&set, SIGIO);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGHUP);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGUSR2);
    sigaddset(&set, SIGWINCH);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGQUIT);

    /**
     * sigprocmask 用于检测或者修改当前的信号集。
     * 参数 SIG_BLOCK 代表 set 的值会被加入到当前屏蔽信号的掩码中。
    */
    if (sigprocmask(SIG_BLOCK, &set, nullptr) == -1)
    {
        xmn_log_info(XMN_LOG_ALERT, errno, "XMNMasterProcessCycle()中 sigprocmask 函数执行失败！");
    }

    /**
     * （2）设置 master 进程标题。
    */
    size_t lenall = 0;
    lenall = g_strmasterprocessname.size();
    std::string strtemp = "";
    for (size_t i = 0; i < g_argc; i++)
    {
        strtemp = g_argv[i];
        g_strmasterprocessname += " " + strtemp;
    }
    XMNSetProcTitle(g_strmasterprocessname);
    xmn_log_info(XMN_LOG_NOTICE, 0, "%s %d 启动成功", g_strmasterprocessname, g_xmn_pid);

    /**
     * （3）创建 worker 子进程。
    */
    XMNConfig *pconfig = SingletonBase<XMNConfig>::GetInstance();
    const size_t kWorkerProcessCount = atoi(pconfig->GetConfigItem("WorkerProcesses", "4").c_str());
    XMNStartWorkerProcess(kWorkerProcessCount);

    /**
     * （4）解除信号屏蔽。
    */
    sigemptyset(&set);

    /**
     * （5）开始 master 进程循环。
    */
    while (true)
    {
        sigsuspend(&set);

        /**
         * TODO：这里添加对各种信号的处理函数。
        */
        sleep(1);
    }
}

static void XMNStartWorkerProcess(const size_t &kWorkerProcessCount)
{
    for (size_t i = 0; i < kWorkerProcessCount; i++)
    {
        if (XMNCreateProcess(i, "worker process") <= 0)
        {
            return;
        }
    }
    return;
}

static int XMNCreateProcess(const size_t &kNum, const std::string &kstrProcName)
{
    int r = 0;
    pid_t pid = fork();
    switch (pid)
    {
    /**
     * 子进程创建失败。
    */
    case -1:
        xmn_log_info(XMN_LOG_ALERT, errno, "XMNCreateProcess fork 产生的子进程num = %d，procname = \"%s\"失败！", kNum, kstrProcName);
        return -1;
    /**
     * 子进程创建成功。
    */
    case 0:
        /**
         * 只有子进程才能运行到这个位置。
        */
        g_xmn_pid_parent = g_xmn_pid;
        g_xmn_pid = getpid();
        r = XMNWorkerProcessCycle(kNum, kstrProcName);
        if (r != 0)
        {
            return -2;
        }

        break;
    default:
        /**
         * 只有父进程才能运行到这个位置。
        */
        break;
    }
    /**
     * 只有父进程才能运行到这个位置。
    */
    return pid;
}

static int XMNWorkerProcessCycle(const size_t &kNum, const std::string &kstrProcName)
{
    int r = 0;
    /**
     * （1）worker 进程初始化。
    */
    g_xmn_process_type = XMN_PROCESS_WORKER;
    r = XMNWorkerProcessInit(kNum, kstrProcName);
    if (r != 0)
    {
        return -1;
    }

    /**
     * （2）开始子进程循环。
    */
    while (true)
    {
        r = XMNProcessEventsTimers();
    }

    /**
     * （3）子进程退出，销毁线程池。
    */
    g_threadpool.Destroy();

    /**
     * （4）socket 中关于子进程部分的变量的销毁。
    */
    g_socket.EndWorker();
    return 0;
}

static int XMNWorkerProcessInit(const size_t &kNum, const std::string &kstrProcName)
{
    /**
     * （1）解锁被屏蔽的信号。
    */
    sigset_t set;
    sigemptyset(&set);
    if (sigprocmask(SIG_SETMASK, &set, nullptr) == -1)
    {
        xmn_log_info(XMN_LOG_ALERT, errno, "XMNWorkerProcessInit 在编号为 %d 的子进程中初始化失败！", kNum);
        return -1;
    }

    /**
     * （2）创建线程池。
    */
    XMNConfig *pconfig = SingletonBase<XMNConfig>::GetInstance();
    const size_t kThreadPoolSize = atoi(pconfig->GetConfigItem("ThreadPoolSize", "100").c_str());

    /**
     * TODO：这里需要判断该函数的返回值。
    */
    if (g_threadpool.Create(kThreadPoolSize))
    {
        return -2;
    }

    /**
     * （3）socket 相关变量初始化。
     * TODO：这里需要判断该函数的返回值。
    */
    if (g_socket.InitializeWorker() != 0)
    {
        return -3;
    }

    /**
     * （4）初始化 epoll ，并向 epoll 添加监听事件。
     * TODO：这里需要判断该函数的返回值。
    */
    const int r = g_socket.EpollInit();
    if (r != 0)
    {
        return -4;
    }

    /**
     * （5）设置进程标题。
    */
    XMNSetProcTitle(kstrProcName);

    xmn_log_info(XMN_LOG_NOTICE, 0, "%s %d 启动成功！", kstrProcName.c_str(), g_xmn_pid);
    return 0;
}