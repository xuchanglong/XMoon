#include "xmoon_config.h"
#include <fstream>

XMoonConfig *XMoonConfig::pinstance_ = nullptr;
XMoonConfig::DeleteXMoonConfig XMoonConfig::delxmoonconfig;

XMoonConfig::XMoonConfig()
{
    ;
}

XMoonConfig::~XMoonConfig()
{
    std::vector<ConfigItem *>::iterator it;
    for (it == vconfig_item_set_.begin(); it != vconfig_item_set_.end(); ++it)
    {
        delete *it;
        *it = nullptr;
    }
    vconfig_item_set_.clear();
}

int XMoonConfig::Load(const std::string &kstrConfigFilePath)
{
    /**
     * 存储返回值。
     */
    int iret = 0;
    /**
     * 打开文件。
     */
    std::ifstream fin(kstrConfigFilePath, std::ios::in);

    /**
     * 循环读取每一行，过滤掉注释等，将配置信息加载到 vconfig_item_set_ 。
     * 检测流上的文件结束符，若检测到，则返回非0，反之则返回0。
     */
    std::string strbuftmp;
    while (getline(fin, strbuftmp))
    {
        /**
         * 去掉注释、空行等无用数据行。
         */
        if (
            (strbuftmp[0] == '#') ||
            (strbuftmp[0] == '\t') ||
            (strbuftmp[0] == '\0') ||
            (strbuftmp[0] == ';') ||
            (strbuftmp[0] == '\n') ||
            (strbuftmp[0] == ' ') ||
            (strbuftmp[0] == '['))
        {
            continue;
        }
        /**
         * 去掉每行后面的换行、回车以及空格等。
         */
        while (true)
        {
            char c = strbuftmp[strbuftmp.size() - 1];
            if ((c == '\n') || (c == '\r') || (c == ' '))
            {
                strbuftmp[strbuftmp.size() - 1] = '\0';
                continue;
            }
            break;
        }
        /**
         * 开始截取选项以及选项信息。
         */
        size_t pos = strbuftmp.find("=");
        if (pos != 0)
        {
            ConfigItem *pitem = new ConfigItem;
            //memset(pitem, 0, sizeof(ConfigItem) * 1);
            pitem->stritem = ClearSpace(strbuftmp.substr(0, pos));
            pitem->striteminfo = ClearSpace(strbuftmp.substr(pos + 1));

            vconfig_item_set_.push_back(pitem);
        }
        strbuftmp.clear();
    }
    fin.clear();
    fin.close();
    return iret;
}

std::string XMoonConfig::GetConfigItem(const std::string &kstrConfigItem)
{
    std::vector<ConfigItem *>::iterator it;
    for (it = vconfig_item_set_.begin(); it != vconfig_item_set_.end(); it++)
    {
        if ((*it)->stritem == kstrConfigItem)
        {
            return (*it)->striteminfo;
        }
    }

    return "";
}

std::string XMoonConfig::ClearSpace(const std::string &kstr)
{
    /**
     * 非空格第1个字符的位置和最后1个字符的位置。
     */
    size_t notspacepos_s = 0;
    size_t notspacepos_e = 0;
    /**
     * 找到字符串从左向右，第一个非空格的字符的位置。
     */
    for (size_t i = 0; i < kstr.size(); ++i)
    {
        if (kstr[i] != ' ')
        {
            notspacepos_s = i;
            break;
        }
    }
    /**
     * 找到字符串从右向左，第一个非空格的字符的位置。
     */
    for (int i = kstr.size() - 1; i >= 0; --i)
    {
        if (kstr[i] != ' ')
        {
            notspacepos_e = i;
            break;
        }
    }
    if (
        ((notspacepos_e - notspacepos_s) < 0)||
        (notspacepos_e==0 && notspacepos_s==0)
        )
    {
        return "";
    }
    return kstr.substr(notspacepos_s, notspacepos_e - notspacepos_s + 1);
}

std::string XMoonConfig::testStringConfigItemSet()
{
    std::string strret;
    strret.clear();

    std::vector<ConfigItem *>::iterator it;
    for (it = vconfig_item_set_.begin(); it != vconfig_item_set_.end(); it++)
    {
        strret += (*it)->stritem + "=" + (*it)->striteminfo + '\n';
    }
    return strret;
}