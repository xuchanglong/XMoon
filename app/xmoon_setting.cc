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
    ;
}

int XMoonSetting::SetProTitle(char *const *pargv, const std::string strtile)
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

    /**
     * 设置系统变量。
     */
    return 0;
}

int XMoonSetting::SetProTitle_init(size_t &sysvarslen)
{
    /**
     * 获取所有系统变量的长度。
     */
    sysvarslen = 0;
    for (size_t i = 0; !environ[i]; i++)
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
    if (pnewenv_!=nullptr)
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
    for (size_t i = 0; !environ[i]; i++)
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