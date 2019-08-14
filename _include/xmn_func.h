//函数声明放在这个头文件里-------------------------------------------

#ifndef XMOON__INCLUDE_XMN_FUNC_H_
#define XMOON__INCLUDE_XMN_FUNC_H_

//字符串相关函数
void   Rtrim(char *string);
void   Ltrim(char *string);

//设置可执行程序标题相关函数
void   xmn_init_setproctitle();
void   xmn_setproctitle(const char *title);

//和日志，打印输出有关
void   xmn_log_init();
void   xmn_log_stderr(int err, const char *fmt, ...);
void   xmn_log_error_core(int level,  int err, const char *fmt, ...);

u_char *xmn_log_errno(u_char *buf, u_char *last, int err);
u_char *xmn_slprintf(u_char *buf, u_char *last, const char *fmt, ...);
u_char *xmn_vslprintf(u_char *buf, u_char *last,const char *fmt,va_list args);

/**
 * 信号相关
*/
int XMNSignalInit();


#endif  