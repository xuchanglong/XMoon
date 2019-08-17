/**
 * @function    设置进程名称的标题的模块。
 * @author       xuchanglong
 * @time            2019-08-17
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "xmn_global.h"

void xmn_setproctitle_init()
{
    g_penvmem = new char[g_envmemlen];
    memset(g_penvmem, 0, g_envmemlen);

    char *ptmp = g_penvmem;
    /**
     * 搬迁环境变量至新位置。
    */
    for (int i = 0; environ[i]; i++)
    {
        size_t size = strlen(environ[i]) + 1;
        strcpy(ptmp, environ[i]);
        environ[i] = ptmp;
        ptmp += size;
    }
    return;
}

int xmn_setproctitle(const std::string &strtitle)
{
    size_t ititlelen = strtitle.size();

    /**
     * 计算命令行和环境变量所占内存的字节数。
    */
    size_t sum = g_argvmemlen + g_envmemlen;
    /**
     * 防止因为标题长度过长导致内存越界。
    */
    if (ititlelen >= sum)
    {
        return -1;
    }
    /**
     * 设置命令行参数只有1个，因为在读取命令行参数时是以是否为空来判断的。
    */
    g_argv[1] = nullptr;

    /**
     * 设置标题。
    */
    char *ptmp = g_argv[0];
    strcpy(ptmp, strtitle.c_str());
    ptmp += ititlelen;

    /**
     *  清空剩余内存。
    */
    size_t len = sum - ititlelen;
    memset(ptmp, 0, len);
    return 0;
}