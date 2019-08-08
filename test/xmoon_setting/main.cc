#include "xmoon_setting.h"
#include <iostream>
#include <unistd.h>

void ShowEnvVars();
void testSetProTitle_init(XMoonSetting &setting);

int main(const int argv, char **argc)
{
    XMoonSetting setting;
    testSetProTitle_init(setting);

    /**
     * 显示进程名的指令：ps -eo pid ,ppid,sid,tty,pgrp,comm,stat,cmd | grep -E "bash|main.o"
     */
    setting.SetProTitle(argc, "XMoon : master process");

    while (true)
    {
        sleep(3);
        std::cout << "sleep 3s" << std::endl;
    }
    
    return 0;
}


void testSetProTitle_init(XMoonSetting &setting)
{
    size_t varslen;

    std::cout << "-----------------  Init  -----------------" << std::endl;
    
    /**
     * 加 void * 的原因是防止 cout 重载进行字符串输出。
     */
    std::cout << "地址：" << (void *)environ[0]<< std::endl;
    ShowEnvVars();

    setting.testSetProTitle_init(varslen);

    std::cout << "-----------------  sec -----------------" << std::endl;

    std::cout << "地址：" << (void *)environ[0] << std::endl;
    ShowEnvVars();
}

void ShowEnvVars()
{
    for (size_t i = 0; environ[i]; i++)
    {
        std::cout <<environ[i] << std::endl;
    }
    
}