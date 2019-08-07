#include "xmoon_config.h"

int testfuncs(std::string strconfigfilepath);

int main()
{
    /**
     * 测试空配置文件。
     */
    std::cout << "--------------  测试空的配置文件！  --------------" << std::endl;
    if (testfuncs("./configfiles/xmoon_emptry.conf") != 0)
    {
        std::cout << "test failed." << std::endl;
    }

    /**
     * 测试正常的配置文件。
     */
    std::cout << std::endl;
    std::cout << "--------------  测试正常的配置文件！  --------------" << std::endl;
    if (testfuncs("./configfiles/xmoon_normal.conf") != 0)
    {
        std::cout << "test failed." << std::endl;
    }
    return 0;
}

int testfuncs(std::string strconfigfilepath)
{
    /**
     * 获取单例。
     */
    XMoonConfig *pconfig = XMoonConfig::GetInstance();
    if (pconfig == nullptr)
    {
        return -1;
    }

    /**
     * 加载配置文件。
     */
    if (pconfig->Load(strconfigfilepath) != 0)
    {
        return -2;
    }

    std::cout << "验证从配置文件中读取的内容是否正确。" << std::endl;
    /**
     * 以字符串的形式获取 XMoonConfig::vconfig_item_set_ 中的内容。
     */
    std::string str = pconfig->testStringConfigItemSet();
    /**
     * 显示。
     */
    std::cout << str << std::endl;

    std::cout << "验证 GetConfigItem 函数是否正确。" << std::endl;
    std::cout << "listenport = " << pconfig->GetConfigItem("listenport") << std::endl;
    std::cout << "logfilename = " << pconfig->GetConfigItem("logfilename") << std::endl;
    std::cout << "loglevel = " << pconfig->GetConfigItem("loglevel") << std::endl;
    std::cout << "workerprocessessum = " << pconfig->GetConfigItem("workerprocessessum") << std::endl;
    std::cout << "runbydaemon = " << pconfig->GetConfigItem("runbydaemon") << std::endl;
    return 0;
}