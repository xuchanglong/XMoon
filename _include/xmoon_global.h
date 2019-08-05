#ifndef XMOON__INCLUDE_XMOONGLOBAL_H_
#define XMOON__INCLUDE_XMOONGLOBAL_H_

#include <iostream>

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

#endif