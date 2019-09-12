/*****************************************************************************************
 * @function    配置文件操作类。包括加载、获取配置信息等。
 * @notice        采用单例模式。
 * @author       xuchanglong
 * @time            2019-08-01    
 *****************************************************************************************/

#ifndef XMOON__INCLUDE_XMNCONFIG_H_
#define XMOON__INCLUDE_XMNCONFIG_H_

#include <vector>
#include <string.h>
#include <new>

#include "xmn_global.h"
#include "base/noncopyable.h"
#include "base/singletonbase.h"

class XMNConfig : public NonCopyable
{
    friend class SingleTonBase<XMNConfig>;

private:
    XMNConfig();
    ~XMNConfig();

public:
    /**
     * @function     加载配置信息。
     * @paras        kstrConfigFilePath   配置文件路径。
     * @return       0   操作成功。
     *               -1  申请内存失败。
     * @author       xuchangong
     * @time          2019-08-01
     */
    int Load(const std::string &kstrConfigFilePath);

    /**
     * @function     得到指定的配置选项的信息。
     * @paras        kstrConfigItem   配置选项。
     *               strdefault      缺省值。若配置文件中不存在该配置选项，那么返回该缺省值。
     * @return       配置选项的信息。未找到则返回缺省值。
     * @author       xuchangong
     * @time         2019-08-01
     */
    std::string GetConfigItem(const std::string &kstrConfigItem, const std::string strdefault = "");

    /**
     * @function     对指定的字符串清空左右的空格字符。
     * @paras        kstr   待处理的字符串。
     * @return       处理完成之后字符串。
     * @author       xuchangong
     * @time         2019-08-06
     */
    std::string ClearSpace(const std::string &kstr);

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
