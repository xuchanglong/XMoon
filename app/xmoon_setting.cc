#include "xmoon_setting.h"
#include <unistd.h>
#include <new>
#include <string.h>

XMoonSetting::XMoonSetting()
{
    pnewenv_ = nullptr;
}

XMoonSetting::~XMoonSetting()
{
    if (pnewenv_ != nullptr)
    {
        delete pnewenv_;
        pnewenv_ = nullptr;
    }
}

int XMoonSetting::SetProTitle(char **&pargv, const std::string strtitle)
{
    if (pargv == nullptr)
    {
        return -1;
    }

    /**
     * 迁移环境变量。
     */
    size_t sysvarslen = 0;
    if (SetProTitle_init(sysvarslen) != 0)
    {
        return -2;
    }

    /**
     * 计算设置的标题长度是否超过命令行和系统变量长度的和。
     */
    size_t paraslen = 0;
    for (size_t i = 0; pargv[i]; i++)
    {
        paraslen += strlen(pargv[i]) + 1;
    }

    size_t titlelen = strtitle.size();
    size_t lentotal = sysvarslen + paraslen;
    if (titlelen >= sysvarslen + paraslen)
    {
        return -3;
    }

    /**
     * 设置进程标题。
     */
    /**
     * 设置后续的命令行参数为空，表示只有argv[]中只有一个元素了.
     * 防止后续argv被滥用，因为很多判断是用argv[] == NULL来做结束标记判断的;
     */
    pargv[1] = nullptr;

    char *ptmp = pargv[0];
    memcpy(ptmp, strtitle.c_str(), titlelen);
    ptmp += titlelen;

    memset(ptmp, 0, lentotal - titlelen);
    return 0;
}

int XMoonSetting::SetProTitle_init(size_t &sysvarslen)
{
    /**
     * 获取所有系统变量的长度。
     */
    sysvarslen = 0;
    for (size_t i = 0; environ[i]; i++)
    {
        sysvarslen += strlen(environ[i]) + 1;
    }
    if (sysvarslen <= 0)
    {
        return -1;
    }

    /**
     * 为所有的系统变量申请新的内存空间。
     */
    if (pnewenv_ != nullptr)
    {
        delete pnewenv_;
        pnewenv_ = nullptr;
    }

    try
    {
        pnewenv_ = new char[sysvarslen]();
    }
    catch (const std::exception &e)
    {
        //std::cerr << e.what() << '\n';
        return -2;
    }

    /**
     * 将系统变量中的内容复制到新的内存空间中，并更新 environ 。
     */
    char *ptmp = pnewenv_;
    for (size_t i = 0; environ[i]; i++)
    {
        size_t len = strlen(environ[i]) + 1;
        memcpy(ptmp, &environ[i], len);
        ptmp += len;
        environ[i] = ptmp;
    }

    return 0;
}

int XMoonSetting::testSetProTitle_init(size_t &sysvarslen)
{
    return SetProTitle_init(sysvarslen);
}