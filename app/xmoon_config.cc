#include "xmoon_config.h"
#include <cstdio>
#include <cstdlib>
#include <sstream>
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
     * 存储配置文件中每一行的数据。
     */
    char linebuf[501] = {'\0'};

    /**
     * 循环读取每一行，过滤掉注释等，将配置信息加载到 vconfig_item_set_ 。
     * 检测流上的文件结束符，若检测到，则返回非0，反之则返回0。
     */
    while (!fin.eof())
    {
        std::string strbuftmp;
        std::getline(fin, strbuftmp);
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
                linebuf[strbuftmp.size() - 1] = '\0';
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
            memset(pitem, 0, sizeof(ConfigItem) * 1);
            pitem->stritem = strbuftmp.substr(0, pos);
            pitem->stritem = strbuftmp.substr(pos + 1);

            vconfig_item_set_.push_back(pitem);
        }
        strbuftmp.clear();
    }
    return iret;
}

std::string XMoonConfig::GetConfigItem(const std::string &kstrConfigItem)
{
    std::string strret = "";

    return strret;
}