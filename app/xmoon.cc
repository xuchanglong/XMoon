#include "xmoon_config.h"
#include "xmoon_setting.h"

int main(const int argc, char *const *argv)
{
    /**
     * 变量初始化
     */

    /**
     * 准备。
     */

    /**
     * 加载配置文件。
     */
    XMoonConfig *pconfig = XMoonConfig::GetInstance();
    if (pconfig==nullptr)
    {
        exit(1);
    }

    /**
     * log 模块初始化。
     */

    /**
     * 创建守护进程。
     */

    /**
     * master 开始工作。
     */

    /**
     * 相关变量内存释放。
     */
    
    return 0;
}