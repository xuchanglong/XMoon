#include "xmoon_setting.h"
#include <iostream>
#include <unistd.h>

void ShowEnvVars();

int main(const int argv, char **argc)
{
    size_t varslen;
    XMoonSetting setting;

    ShowEnvVars();

    setting.testSetProTitle_init(varslen);

    ShowEnvVars();

    return 0;
}

void ShowEnvVars()
{
    for (size_t i = 0; !environ[i]; i++)
    {
        std::cout << environ[i] << std::endl;
    }
    
}