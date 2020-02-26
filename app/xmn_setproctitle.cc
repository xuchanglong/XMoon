/*****************************************************************************************
 * @function    设置进程名称的标题的模块。
 * @time    2019-08-17
*****************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "xmn_global.h"

void XMNSetProcTitleInit()
{
    if (g_envmemlen <= 0)
    {
        return;
    }

    g_penvmem = new char[g_envmemlen];
    memset(g_penvmem, '\0', g_envmemlen);

    char *ptmp = g_penvmem;

    /**
     * 搬迁环境变量至新位置。
    */
    for (size_t i = 0; environ[i]; i++)
    {
        size_t size = strlen(environ[i]) + 1;
        memcpy(ptmp, environ[i], size);
        environ[i] = ptmp;
        ptmp += size;
    }
    return;
}

int XMNSetProcTitle(const std::string &kstrTitle)
{
    const size_t kTitleLen = kstrTitle.size();

    /**
     * （1）计算命令行和环境变量所占内存的字节数。
    */
    size_t sum = g_argvmemlen + g_envmemlen;

    /**
     * （2）防止因为标题长度过长导致内存越界。
    */
    if (kTitleLen >= sum)
    {
        return -1;
    }

    /**
     * （3）设置命令行参数只有 1 个，因为在读取命令行参数时是以是否为空来判断的。
    */
    g_argv[1] = nullptr;

    /**
     * （4）设置标题。
    */
    char *ptmp = g_argv[0];
    memcpy(ptmp, kstrTitle.c_str(), kTitleLen + 1);
    ptmp += kTitleLen + 1;

    /**
     *  （5）清零剩余内存。
    */
    size_t len = sum - kTitleLen - 1;
    memset(ptmp, '\0', len);
    return 0;
}