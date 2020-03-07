/*****************************************************************************************
 * @function    与日志相关。
 * @time    2019-08-15
*****************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>   
#include <stdarg.h>   
#include <unistd.h>   
#include <sys/time.h> 
#include <time.h>     
#include <fcntl.h>    
#include <errno.h>    

#include "xmn_global.h"
#include "xmn_macro.h"
#include "xmn_func.h"
#include "xmn_config.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * 错误等级，和xmn_macro.h里定义的日志等级宏是一一对应关系。
*/
static u_char err_levels[][20] =
    {
        {"stderr"}, //0：控制台错误
        {"emerg"},  //1：紧急
        {"alert"},  //2：警戒
        {"crit"},   //3：严重
        {"error"},  //4：错误
        {"warn"},   //5：警告
        {"notice"}, //6：注意
        {"info"},   //7：信息
        {"debug"}   //8：调试
};
XMNLog g_xmn_log;

void XMNLogStdErr(int err, const char *fmt, ...)
{
    va_list args;                      
    u_char errstr[XMN_MAX_ERROR_STR + 1]; 
    u_char *p, *last;

    memset(errstr, 0, sizeof(errstr)); 

    last = errstr + XMN_MAX_ERROR_STR; 

    p = XMN_CPYMEM(errstr, "xmoon: ", 7); 

    va_start(args, fmt);                   
    p = xmn_vslprintf(p, last, fmt, args); 
    va_end(args);                    

    if (err)
    {
        p = xmn_log_errno(p, last, err);
    }

    if (p >= (last - 1))
    {
        p = (last - 1) - 1; 
                            
    }
    *p++ = '\n'; 

    write(STDERR_FILENO, errstr, p - errstr); 

    if (g_xmn_log.fd > STDERR_FILENO)
    {
        XMNLogInfo(XMN_LOG_STDERR, err, (const char *)errstr);
    }

    return;
}

u_char *xmn_log_errno(u_char *buf, u_char *last, int err)
{
    char *perrorinfo = strerror(err);
    size_t len = strlen(perrorinfo);

    char leftstr[10] = {0};
    sprintf(leftstr, " (%d: ", err);
    size_t leftlen = strlen(leftstr);

    char rightstr[] = ") ";
    size_t rightlen = strlen(rightstr);

    size_t extralen = leftlen + rightlen; 
    if ((buf + len + extralen) < last)
    {
        buf = XMN_CPYMEM(buf, leftstr, leftlen);
        buf = XMN_CPYMEM(buf, perrorinfo, len);
        buf = XMN_CPYMEM(buf, rightstr, rightlen);
    }
    return buf;
}

void XMNLogInfo(int level, int err, const char *fmt, ...)
{
    u_char *last;
    u_char errstr[XMN_MAX_ERROR_STR + 1]; 

    memset(errstr, 0, sizeof(errstr));
    last = errstr + XMN_MAX_ERROR_STR;

    struct timeval tv;
    struct tm tm;
    time_t sec; 
    u_char *p; 
    va_list args;

    memset(&tv, 0, sizeof(struct timeval));
    memset(&tm, 0, sizeof(struct tm));

    gettimeofday(&tv, NULL); 

    sec = tv.tv_sec;     
    localtime_r(&sec, &tm); 
    tm.tm_mon++;        
    tm.tm_year += 1900;    

    u_char strcurrtime[40] = {0}; 
    xmn_slprintf(strcurrtime,
                 (u_char *)-1,                 
                 "%4d/%02d/%02d %02d:%02d:%02d", 
                 tm.tm_year, tm.tm_mon,
                 tm.tm_mday, tm.tm_hour,
                 tm.tm_min, tm.tm_sec);
    p = XMN_CPYMEM(errstr, strcurrtime, strlen((const char *)strcurrtime)); 
    p = xmn_slprintf(p, last, " [%s] ", err_levels[level]);               
    p = xmn_slprintf(p, last, "%P: ", g_xmn_pid);                    

    va_start(args, fmt);                
    p = xmn_vslprintf(p, last, fmt, args);
    va_end(args);                      

    if (err) 
    {
  
        p = xmn_log_errno(p, last, err);
    }

    if (p >= (last - 1))
    {
        p = (last - 1) - 1; 
                           
    }
    *p++ = '\n'; 
    
    ssize_t n;
    while (1)
    {
        if (level > g_xmn_log.log_level)
        {
            break;
        }

        n = write(g_xmn_log.fd, errstr, p - errstr);
        if (n == -1)
        {
            if (errno == ENOSPC) 
            {
            }
            else
            {
                if (g_xmn_log.fd != STDERR_FILENO)
                {
                    n = write(STDERR_FILENO, errstr, p - errstr);
                }
            }
        }
        break;
    } //end while
    return;
}

int XMNLogInit()
{
    XMNConfig &config = SingletonBase<XMNConfig>::GetInstance();
    std::string strplogname = config.GetConfigItem("LogFileName");
    if (strplogname.empty())
    {
        strplogname = XMN_ERROR_LOG_PATH; 
    }
    g_xmn_log.log_level = std::stoi(config.GetConfigItem("LogLevel", std::to_string(XMN_LOG_NOTICE))); 

    g_xmn_log.fd = open(strplogname.c_str(), O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (g_xmn_log.fd == -1) 
    {
        XMNLogStdErr(errno, "[alert] could not open error log file: open() \"%s\" failed", strplogname.c_str());
        g_xmn_log.fd = STDERR_FILENO; 
        return -1;
    }
    return 0;
}
