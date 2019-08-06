#include "xmoon_config.h"

int main()
{
    /**
     * 获取单例。
     */
    XMoonConfig *pconfig = XMoonConfig::GetInstance();
    /**
     * 加载配置文件。
     */
    pconfig->Load("../../xmoon.conf");
    /**
     * 以字符串的形式获取 XMoonConfig::vconfig_item_set_ 中的内容。
     */
    std::string str = pconfig->testStringConfigItemSet();
    /**
     * 显示。
     */
    std::cout << str << std::endl;
    return 0;
}