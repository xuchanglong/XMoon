/**
 * @function    存放全局函数。
 * @author       xuchanglong
 * @time            2019-08-15
*/

#ifndef XMOON__INCLUDE_XMN_FUNC_H_
#define XMOON__INCLUDE_XMN_FUNC_H_

/**
 * 设置进程相关函数。
*/
void xmn_init_setproctitle();
void xmn_setproctitle(const char *title);

/**
 * 日志打印相关函数。
*/
void xmn_log_init();
void xmn_log_stderr(int err, const char *fmt, ...);
void xmn_log_error_core(int level, int err, const char *fmt, ...);

u_char *xmn_log_errno(u_char *buf, u_char *last, int err);
u_char *xmn_slprintf(u_char *buf, u_char *last, const char *fmt, ...);
u_char *xmn_vslprintf(u_char *buf, u_char *last, const char *fmt, va_list args);

/**
 * 信号相关
*/
int XMNSignalInit();

/**
 * 主流程相关函数。
*/
void xmn_master_process_cycle();

#endif