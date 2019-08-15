/**
 * @function    整个项目的入口函数。
 * @author        xuchanglong
 * @time            2019-08-14
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "xmn_config.h"
#include "xmn_func.h"
#include "xmn_macro.h"

/**
 * @function    释放内存。
 * @paras           none 。
 * @author          xuchanglong
 * @time            2019-08-14
*/
static void freeresource();

size_t g_argvmemlen = 0;
size_t g_envmemlen = 0;
char **g_argv = nullptr;
size_t g_argc = 0;
char *gp_envmem = nullptr;

pid_t xmn_pid = -1;
pid_t xmn_pid_parent = -1;
int xmn_process = XMN_PROCESS_MASTER;

int main(int argc, char *const *argv)
{
    /**
     *  变量初始化。 
     */
    int exitcode = 0;
    xmn_pid = getpid();
    xmn_pid = getppid();
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
        xmn_log_stderr(0, "配置文件[%s]载入失败，退出!", "xmoon.conf");
        exitcode = 1;
        goto lblexit;
    }

    /**
     * 初始化日志模块。
    */
    xmn_log_init();

    /**
     * 初始化信号模块。
    */
    if (XMNSignalInit() == -1)
    {
        exitcode = 1;
        goto lblexit;
    }

    /**
     * 初始化设置程序名称模块。
    */
    xmn_init_setproctitle();

    /**
     * 开始进入主进程工作流程。
    */
    xmn_master_process_cycle();
    //--------------------------------------
lblexit:
    /**
     *  释放内存。
    */
    freeresource();
    printf("程序退出，再见!\n");
    return exitcode;
}

//专门在程序执行末尾释放资源的函数【一系列的main返回前的释放动作函数】
void freeresource()
{
    //(1)对于因为设置可执行程序标题导致的环境变量分配的内存，我们应该释放
    if (gp_envmem)
    {
        delete[] gp_envmem;
        gp_envmem = NULL;
    }

    //(2)关闭日志文件
    if (xmn_log.fd != STDERR_FILENO && xmn_log.fd != -1)
    {
        close(xmn_log.fd); //不用判断结果了
        xmn_log.fd = -1;   //标记下，防止被再次close吧
    }
}
