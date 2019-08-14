#include <signal.h>
#include <iostream>
#include <errno.h>
#include "xmn_macro.h"
#include "xmn_func.h"
#include "xmn_global.h"
#include "xmn_config.h"

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
 * @return          none 。
 * @author          xuchanglong
 * @time              2019-08-15
*/
static void xmn_create_process(const size_t &inum, const std::string &strprocname);

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
    * 设置master进程名称。
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
        ;
    }
}

static void xmn_start_worker_process(const size_t &workerprocesssum)
{
    ;
}

static void xmn_create_process(const size_t &inum, const std::string &strprocname)
{
    ;
}

static void xmn_worker_process_cycle(const size_t &inum, const std::string &strprocname)
{
    ;
}

static void xmn_worker_process_init(const size_t &inum)
{
    ;
}