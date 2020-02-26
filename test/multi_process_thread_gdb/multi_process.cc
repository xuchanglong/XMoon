/*****************************************************************************************
 * @function    学习 gdb 调试多进程方法的测试代码。
 * @notice  采用单例模式。
 * @time    2019-09-06
 *****************************************************************************************/

#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
    pid_t pid = fork();

    if (pid == -1)
    {
        std::cout << "fork failed." << std::endl;
        return -1;
    }
    else if (pid == 0)
    {
        std::cout << "I am a child ,my pid is " << getpid() << ",my father's pid is " << getppid() << "." << std::endl;
    }
    else
    {
        std::cout << "I am a father ,my pid is " << getpid() << "." << std::endl;
        /**
         * 父进程堵塞在这里，等待子进程退出。
        */
        wait(NULL);
    }

    return 0;
}