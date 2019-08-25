/*****************************************************************************************
 * @function   创建守护进程的功能函数。
 * @author      xuchanglong
 * @time            2019-08-15
*****************************************************************************************/
#include "unistd.h"
#include "xmn_func.h"
#include "xmn_global.h"
#include "xmn_macro.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "errno.h"
#include "fcntl.h"

int xmn_daemon()
{
    /**
     * 创建进程。
    */
    switch (fork())
    {
    /**
     * 创建进程失败。
    */
    case -1:
        xmn_log_info(XMN_LOG_ALERT, errno, "%s %s %d fork() failed.", __FILE__, __FUNCTION__, __LINE__);
        return -1;

    /**
     * 子进程。
    */
    case 0:
        break;

    /**
     * 父进程。
    */
    default:
        return 1;
    }
    g_xmn_pid_parent = g_xmn_pid;
    g_xmn_pid = getpid();

    /**
     * 1、使该进程成为新会话的首进程。
     * 2、使该进程成为新进程组的组长进程。
     * 3、使该进程脱离终端。
     * 
     * 该函数返回值为-1的情况是该进程是该会话的首进程。
    */
    if (setsid() == -1)
    {
        xmn_log_info(XMN_LOG_EMERG, errno, "%s %s %d setsid() failed.", __FILE__, __FUNCTION__, __LINE__);
        return -2;
    }

    /**
     * 设置该进程创建的文件和目录的权限不受制约。
    */
    umask(0);

    /**
     * 将标准的读写方式设置为黑洞。
    */
    int fd = open("/dev/null", O_RDWR);
    if (fd == -1)
    {
        xmn_log_info(XMN_LOG_EMERG, errno, "%s %s %d open(\"/dev/null\") failed.", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    if (dup2(fd, STDIN_FILENO) == -1)
    {
        xmn_log_info(XMN_LOG_EMERG, errno, "%s %s %d dup2(STDIN) failed.", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    if (dup2(fd, STDOUT_FILENO) == -1)
    {
        xmn_log_info(XMN_LOG_EMERG, errno, "%s %s %d dup2(STDOUT) failed.", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    if (fd > STDERR_FILENO)
    {
        if (close(fd) == -1)
        {
            xmn_log_info(XMN_LOG_EMERG, errno, "%s %s %d close(fd) failed.", __FILE__, __FUNCTION__, __LINE__);
            return -1;
        }
    }

    return 0;
}