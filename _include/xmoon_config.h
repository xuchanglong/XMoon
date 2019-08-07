/**
 * @function    配置文件操作类。包括加载、获取配置信息等。
 * @notice        采用单例模式。
 * @author       xuchanglong
 * @time            2019-08-01    
 */

#ifndef XMOON__INCLUDE_XMOONCONFIG_H_
#define XMOON__INCLUDE_XMOONCONFIG_H_

#include "xmoon_global.h"
#include <vector>
#include <string.h>
#include <new>

class XMoonConfig
{
private:
    XMoonConfig();
    ~XMoonConfig();

public:
    /**
     * @function    单例的生成器。
     * @paras           none
     * @return         单例的指针。
     * @author        xuchanglong
     * @time             2019-08-01        
     */
    static XMoonConfig *GetInstance()
    {
        if (pinstance_ == nullptr)
        {
            if (pinstance_ == nullptr)
            {
                try
                {
                    pinstance_ = new XMoonConfig();
                }
                catch(const std::exception& e)
                {
                    //std::cerr << e.what() << '\n';
                    return nullptr;
                }
            }
        }
        return pinstance_;
    }

private:
    /**
     * @function    单例的销毁类。
     * @author        xuchanglong
     * @time             2019-08-01        
     */
    class DeleteXMoonConfig
    {
    public:
        ~DeleteXMoonConfig()
        {
            if (XMoonConfig::pinstance_)
            {
                delete XMoonConfig::pinstance_;
                XMoonConfig::pinstance_ = nullptr;
            }
        }
    };

public:
    /**
     * @function     加载配置信息。
     * @paras           kstrConfigFilePath   配置文件路径。
     * @return          0   操作成功。
     *                  -1  申请内存失败。
     * @author        xuchangong
     * @time            2019-08-01
     */
    int Load(const std::string &kstrConfigFilePath);

    /**
     * @function     得到指定的配置选项的信息。
     * @paras           kstrConfigItem   配置选项。
     * @return         配置选项的信息。未找到则返回空。
     * @author        xuchangong
     * @time            2019-08-01
     */
    std::string GetConfigItem(const std::string &kstrConfigItem);

    /**
     * @function     对指定的字符串清空左右的空格字符。
     * @paras        kstr   待处理的字符串。
     * @return       处理完成之后字符串。
     * @author        xuchangong
     * @time            2019-08-06
     */
    std::string ClearSpace(const std::string &kstr);

private:
    static XMoonConfig *pinstance_;
    static DeleteXMoonConfig delxmoonconfig;

private:
    /**
     * 配置文件中所有条目的信息的集合。
     */
    std::vector<ConfigItem *> vconfig_item_set_;
/**
 * 测试接口,正常使用时禁止使用。
 */
public:
    /**
     * @function    返回 vconfig_item_set_ 选项和值的字符串形式。
     * @paras       none 。
     * @return      vconfig_item_set_ 选项和值的字符串形式。
     * @author      xuchanglong
     * @time        2019-08-06
     */
    std::string testStringConfigItemSet();
};

#endif
