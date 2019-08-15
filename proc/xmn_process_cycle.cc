#include <signal.h>
#include <iostream>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include "xmn_macro.h"
#include "xmn_func.h"
#include "xmn_global.h"
#include "xmn_config.h"

/**
 * master 进程标题。
*/
static std::string g_strmasterprocessname = "master process";

/**
 * @function    开始处理 workers 进程。
 * @paras           workprocesssum  创建 worker 进程的数量。
 * @return          none 。
 * @author          xuchanglong
 * @time              2019-08-15
*/
static void xmn_start_worker_process(const size_t &workerprocesssum);

/**
 * @function    创建指定数量的 worker 进程。
 * @paras           inum    子进程编号。
 *                            strprocname   子进程的名称。
 * @return          创建的子进程的 pid。
 * @author          xuchanglong
 * @time              2019-08-15
*/
static int xmn_create_process(const size_t &inum, const std::string &strprocname);

/**
 * @function    设置子进程的标题以及进入子进程的循环。
 * @paras           inum    子进程编号。
 *                            strprocname   子进程的名称。
 * @return          none 。
 * @author          xuchanglong
 * @time              2019-08-15
*/
static void xmn_worker_process_cycle(const size_t &inum, const std::string &strprocname);

/**
 * @function    子进程开始进入循环之前进行初始化。
 * @paras           inum    子进程编号。
 * @return          none 。
 * @author          xuchanglong
 * @time              2019-08-15
*/
static void xmn_worker_process_init(const size_t &inum);

void xmn_master_process_cycle()
{
    /**
     * 屏蔽各种信号。
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

    if (sigprocmask(SIG_BLOCK, &set, nullptr) == -1)
    {
        xmn_log_error_core(XMN_LOG_ALERT, errno, "xmn_master_process_cycle()中sigprocmask函数执行失败！");
    }

    /**
    * 设置master进程标题。
   */
    size_t lenall = 0;
    lenall = g_strmasterprocessname.size();
    lenall += g_envmemlen;
    std::string strtemp = "";
    if (lenall < 1000)
    {
        for (size_t i = 0; i < g_argc; i++)
        {
            strtemp = g_argv[i];
            g_strmasterprocessname += +" " + strtemp;
        }
    }
    xmn_setproctitle(g_strmasterprocessname.c_str());

    /**
   * 创建 worker 子进程。
  */
    XMNConfig *pconfig = XMNConfig::GetInstance();
    size_t iworkerprocesssum = atoi(pconfig->GetConfigItem("WorkderProcess", "4").c_str());
    /**
     * 解除信号屏蔽。
    */
    sigemptyset(&set);

    /**
    * 开始master进程循环。
   */
    while (true)
    {
        std::cout << " master 进程运行。" << std::endl;
        sleep(1);
    }
}

static void xmn_start_worker_process(const size_t &workerprocesssum)
{
    for (size_t i = 0; i < workerprocesssum; i++)
    {
        xmn_create_process(i, "worker process");
    }
    return;
}

static int xmn_create_process(const size_t &inum, const std::string &strprocname)
{
    pid_t pid = fork();
    switch (pid)
    {
    /**
     * 子进程创建失败。
    */
    case -1:
        xmn_log_error_core(XMN_LOG_ALERT, errno, "xmn_create_process fork 产生的子进程num = %d，procname = \"%s\"失败！", inum, strprocname);
        break;
    /**
     * 子进程创建成功。
    */
    case 0:
        /**
         * 只有子进程才能运行到这个位置。
        */
        xmn_pid_parent = xmn_pid;
        xmn_pid = getpid();
        xmn_worker_process_cycle(inum, strprocname);
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

static void xmn_worker_process_cycle(const size_t &inum, const std::string &strprocname)
{
    xmn_process = XMN_PROCESS_WORKER;
    xmn_worker_process_init(inum);
    xmn_setproctitle(strprocname.c_str());
    xmn_log_error_core(XMN_LOG_NOTICE, errno, "%s %d 启动成功！", strprocname.c_str(), xmn_pid);
    while (true)
    {
        std::cout << inum << "  子进程运行。" << std::endl;
        sleep(1);
    }
}

static void xmn_worker_process_init(const size_t &inum)
{
    sigset_t set;
    sigemptyset(&set);
    if (sigprocmask(SIG_SETMASK, &set, nullptr) == -1)
    {
        xmn_log_error_core(XMN_LOG_ALERT, errno, "xmn_worker_process_init 在编号为 %d 的子进程中初始化失败！", inum);
    }
}