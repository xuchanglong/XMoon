/*****************************************************************************************
 * @function    整个项目的入口函数。
 * @author      xuchanglong
 * @time        2019-08-14
*****************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "xmn_config.h"
#include "xmn_func.h"
#include "xmn_macro.h"
#include "xmn_socket.h"
#include "xmn_memory.h"

/**
 * @function    释放为搬迁环境变量而申请的内存以及关闭日志文件句柄。
 * @paras           none 。
 * @author          xuchanglong
 * @time            2019-08-14
*/
static void freeresource();

size_t g_argvmemlen = 0;
size_t g_envmemlen = 0;
char **g_argv = nullptr;
size_t g_argc = 0;
char *g_penvmem = nullptr;
bool g_isdaemonized = 0;

XMNSocket g_socket;

pid_t g_xmn_pid = -1;
pid_t g_xmn_pid_parent = -1;
int g_xmn_process_type = XMN_PROCESS_MASTER;

sig_atomic_t g_xmn_reap = 0;

int main(int argc, char *const *argv)
{
    /**
     *  变量初始化。 
     */
    std::string strdaemoncontext;
    int exitcode = 0;
    g_xmn_pid = getpid();
    g_xmn_pid_parent = getppid();
    g_argv = (char **)argv;

    /**
     * 统计 argv 和 env 所占的内存。
    */
    g_argvmemlen = 0;
    for (size_t i = 0; i < argc; i++)
    {
        g_argvmemlen += strlen(argv[i]) + 1;
    }

    g_envmemlen = 0;
    for (size_t i = 0; environ[i]; i++)
    {
        g_envmemlen += strlen(environ[i]) + 1;
    }

    g_xmn_log.fd = -1;
    g_xmn_log.log_level = 8;

    /**
    * 保存参数个数和指针。
    */
    g_argc = argc;
    g_argv = (char **)argv;

    /**
     * 初始化配置模块。
    */
    XMNConfig *p_config = XMNConfig::GetInstance();
    if (p_config->Load("xmoon.conf") != 0)
    {
        xmn_log_init();
        xmn_log_stderr(0, "配置文件[%s]载入失败，退出!", "xmoon.conf");
        exitcode = 1;
        goto lblexit;
    }
    /**
     * 单例 XMNMemory 初始化。
    */
    XMNMemory::GetInstance();

    /**
     * 初始化日志模块。
    */
    xmn_log_init();

    /**
     * 初始化信号模块。
    */
    if (XMNSignalInit() == -1)
    {
        exitcode = 2;
        goto lblexit;
    }

    /**
     * 开始监听指定 port 。
    */
    if (g_socket.Initialize() != 0)
    {
        exitcode = 3;
        goto lblexit;
    }

    /**
     * 初始化设置程序名称模块。
    */
    xmn_setproctitle_init();

    /**
     * 创建守护进程。
    */
    strdaemoncontext = p_config->GetConfigItem("Daemon", "0");
    if (strdaemoncontext.compare("1"))
    {
        int r = xmn_daemon();

        /**
         * 父进程退出。
        */
        if (r == 1)
        {
            freeresource();
            exitcode = 0;
            return exitcode;
        }

        /**
         * 创建进程失败。
        */
        else if (r != 0)
        {
            exitcode = 3;
            goto lblexit;
        }

        /**
         * 守护进程创建成功。
        */
        g_isdaemonized = true;
    }

    /**
     * 开始进入主进程工作流程。
    */
    xmn_master_process_cycle();
lblexit:
    /**
     *  释放内存。
    */
    freeresource();
    printf("程序退出，再见!\n");
    return exitcode;
}

void freeresource()
{
    /**
     * 释放存储的环境变量。
    */
    if (g_penvmem)
    {
        delete[] g_penvmem;
        g_penvmem = nullptr;
    }

    /**
     * 关闭日志文件。
    */
    if (g_xmn_log.fd != STDERR_FILENO && g_xmn_log.fd != -1)
    {
        close(g_xmn_log.fd);
        g_xmn_log.fd = -1;
    }
}
