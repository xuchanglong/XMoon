/**
 * @function    进程的相关设置的类。
 * @author      xuchanglong
 * @time        2019-08-08
 */
#ifndef XMOON__INCLUDE_XMOON_SETTING_H_
#define XMOON__INCLUDE_XMOON_SETTING_H_

#include <string>

class XMoonSetting
{
public:
    XMoonSetting();
    XMoonSetting(const XMoonSetting &obj) = delete;
    XMoonSetting &operator=(const XMoonSetting &obj) = delete;
    virtual ~XMoonSetting();
public:
    /**
     * @function    设置进程的标题。
     * @paras       标题名称。
     * @return      
     * @author      xuchanglong
     * @time        2019-08-08
     */
    int SetProTitle(char **&pargv, const std::string strtile);
private:
    /**
     * @function    设置进程的标题功能的初始化函数。
     * @paras       
     * @return      
     * @author      xuchanglong
     * @time        2019-08-08
     */
    int SetProTitle_init(size_t &sysvarslen);
private:
    /**
     * 
     */
    char *pnewenv_;

/**
 * 测试接口,正常使用时禁止使用。
 */
public:
    int testSetProTitle_init(size_t &sysvarslen);
};

#endif