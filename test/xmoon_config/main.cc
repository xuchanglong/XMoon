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

    //std::cout << "验证从配置文件中读取的内容是否正确。" << std::endl;
    /**
     * 以字符串的形式获取 XMoonConfig::vconfig_item_set_ 中的内容。
     */
    //std::string str = pconfig->testStringConfigItemSet();
    /**
     * 显示。
     */
    //std::cout << str << std::endl;

    //std::cout << "验证 GetConfigItem 函数是否正确。" << std::endl;
    //std::cout << "listenport = " << pconfig->GetConfigItem("listenport") << std::endl;
    //std::cout << "logfilename = " << pconfig->GetConfigItem("logfilename") << std::endl;
    //std::cout << "loglevel = " << pconfig->GetConfigItem("loglevel") << std::endl;
    //std::cout << "workerprocessessum = " << pconfig->GetConfigItem("workerprocessessum") << std::endl;
    //std::cout << "runbydaemon = " << pconfig->GetConfigItem("runbydaemon") << std::endl;

    return 0;
}