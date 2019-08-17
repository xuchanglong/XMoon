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
     * 脱离终端。
    */
    if (setsid() == -1)
    {
        xmn_log_info(XMN_LOG_EMERG, errno, "%s %s %d setsid() failed.", __FILE__, __FUNCTION__, __LINE__);
        return -2;
    }

    umask(0);

    /**
     * 讲标准的读写方式设置为黑洞。
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