#include "xmoon_config.h"

int main()
{
    XMoonConfig *pconfig = XMoonConfig::GetInstance();
    pconfig->Load("../../xmoon.conf");
    return 0;
}