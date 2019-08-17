/**
 * @function    各种宏存放的地方。
 * @author      xuchanglong
 * @time            2019-08-15
*/
#ifndef XMOON__INCLUDE_XMN_MACRO_H_
#define XMOON__INCLUDE_XMN_MACRO_H_

/**
 * 显示的错误信息最大数组长度
*/
#define XMN_MAX_ERROR_STR 2048

//简单功能函数--------------------
//类似memcpy，但常规memcpy返回的是指向目标dst的指针，而这个xmn_cpymem返回的是目标【拷贝数据后】的终点位置，连续复制多段数据时方便
#define xmn_cpymem(dst, src, n) (((u_char *)memcpy(dst, src, n)) + (n)) //注意#define写法，n这里用()包着，防止出现什么错误
#define xmn_min(val1, val2) ((val1 > val2) ? (val2) : (val1))           //比较大小，返回小值，注意，参数都用()包着

//最大的32位无符号数：十进制是‭4294967295‬
#define XMN_MAX_UINT32_VALUE (uint32_t)-1 
#define XMN_INT64_LEN (sizeof("-9223372036854775808") - 1)

/**
 * 声明日志等级，从低到高，日志等级依次增强。
*/
#define XMN_LOG_STDERR 0 //控制台错误【stderr】：最高级别日志，日志的内容不再写入log参数指定的文件，而是会直接将日志输出到标准错误设备比如控制台屏幕
#define XMN_LOG_EMERG 1  //紧急 【emerg】
#define XMN_LOG_ALERT 2  //警戒 【alert】
#define XMN_LOG_CRIT 3   //严重 【crit】
#define XMN_LOG_ERR 4    //错误 【error】：属于常用级别
#define XMN_LOG_WARN 5   //警告 【warn】：属于常用级别
#define XMN_LOG_NOTICE 6 //注意 【notice】
#define XMN_LOG_INFO 7   //信息 【info】
#define XMN_LOG_DEBUG 8  //调试 【debug】：最低级别

/**
 * 设置日志存放路径以及文件名称。
*/
#define XMN_ERROR_LOG_PATH "error.log"

/**
 * 进程类型。
*/
#define XMN_PROCESS_MASTER 1
#define XMN_PROCESS_WORKER 2

#endif
