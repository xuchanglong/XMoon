/**
 * @function    存放声明的全局变量、结构体和宏等相关信息。
 * @author       xuchanglong
 * @time            2019-08-14
*/
#ifndef XMOON__INCLUDE_XMN_GLOBAL_H_
#define XMOON__INCLUDE_XMN_GLOBAL_H_

#include <iostream>

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

//和运行日志相关
typedef struct
{
    int log_level; //日志级别 或者日志类型，xmn_macro.h里分0-8共9个级别
    int fd;        //日志文件描述符

} xmn_log_t;

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
extern int g_argc;
/**
 * 新搬家的环境变量的存放位置。
 * 在 在xmn_init_setproctitle 中申请内存。
*/
extern char *gp_envmem;

/**
 * 当前进程的 pid （master or work）。
*/
extern pid_t xmn_pid;
/**
 * 父进程的 pid，一般work进程调用，存储 master 进程的 pid 。
*/
extern pid_t xmn_pid_parent;

/**
 * 进程类型，记录当前的进程是 master 还是 worker 。 
*/
extern int xmn_process;
extern xmn_log_t xmn_log;

#endif
