/*****************************************************************************************
 * @function    存放声明的全局变量、结构体等相关信息。
 * @author       xuchanglong
 * @time            2019-08-14
*****************************************************************************************/
#ifndef XMOON__INCLUDE_XMN_GLOBAL_H_
#define XMOON__INCLUDE_XMN_GLOBAL_H_

#include <string>
#include "signal.h"
#include "xmn_socket.h"
#include "xmn_threadpool.h"

/**
 * @function    记录配置文件中每一个条目的信息。
 * @author        xuchanglong
 * @time            2019-08-01
 */
struct ConfigItem
{
    /**
     * 条目名称。
     */
    std::string stritem;
    /**
     * 条目的配置信息。
     */
    std::string striteminfo;
};

/**
 * @function    记录打印的日志的相关信息。
 * @author         xuchanglong
 * @time            2019-08-17
*/
struct XMNLog
{
    /**
     * 打印的日志的最低等级。
    */
    int log_level;
    /**
     * 日志文件的文件描述符。
    */
    int fd;
};

/**
 * argv 参数所占内存大小。
*/
extern size_t g_argvmemlen;

/**
 * 环境变量所占内存大小。
*/
extern size_t g_envmemlen;

/**
 * 存放 argv 参数的首地址。
*/
extern char **g_argv;

/**
 *   argv 参数个数。
 */
extern size_t g_argc;

/**
 * 新搬家的环境变量的存放位置。
 * 在 xmn_setproctitle_init 中申请内存。
*/
extern char *g_penvmem;

/**
 * 当前进程的 pid （master or work）。
*/
extern pid_t g_xmn_pid;

/**
 * 父进程的 pid，一般work进程调用，存储 master 进程的 pid 。
*/
extern pid_t g_xmn_pid_parent;

/**
 * 进程类型，记录当前的进程是 master 还是 worker 。 
*/
extern int g_xmn_process_type;

/**
 * 记录打开的日志文件的相关信息。
*/
extern XMNLog g_xmn_log;

/**
 * 守护进程是否启动成功。
 * 0    未启用。
 * 1    已启用。
*/
extern bool g_isdaemonized;

/**
 * 标记子进程状态变化。
 * 0    没有变化。
 *  1   已变化。
*/
extern sig_atomic_t g_xmn_reap;

/**
 * xmn socket 对象。
*/
extern XMNSocket g_socket;

/**
 * 线程池对象。
*/
extern XMNThreadPool g_threadpool;
#endif
