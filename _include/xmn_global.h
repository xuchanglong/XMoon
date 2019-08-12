
#ifndef XMOON__INCLUDE_XMN_GLOBAL_H_
#define XMOON__INCLUDE_XMN_GLOBAL_H_

#include <iostream>

//一些比较通用的定义放在这里，比如typedef定义
//一些全局变量的外部声明也放在这里

//类型定义----------------

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
} ;

//和运行日志相关 
typedef struct
{
	int    log_level;   //日志级别 或者日志类型，xmn_macro.h里分0-8共9个级别
	int    fd;          //日志文件描述符

}xmn_log_t;


//外部全局量声明
extern char  **g_os_argv;
extern char  *gp_envmem; 
extern int   g_environlen; 

extern pid_t       xmn_pid;
extern xmn_log_t   xmn_log;

#endif
