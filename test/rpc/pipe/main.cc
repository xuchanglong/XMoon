/*****************************************************************************************
 * @function    无名管道示例代码。
 * @notice  fd1，parent 写，child 读。
 *          fd2，child 写，parent 读。
 * @time    2019-09-22    
 *****************************************************************************************/
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include <unistd.h>

size_t strlen(char *pdata)
{
    const char *ptmp = pdata;
    while (*pdata != '\0')
        pdata++;
    return pdata - ptmp;
}

void parent(int writefd, int readfd)
{
    char sendbuf[128] = "Hello child";
    char recvbuf[128];
    write(writefd, sendbuf, strlen(sendbuf) + 1);
    read(readfd, recvbuf, 128);
    std::cout << "parent process read from pipe is \"" << recvbuf << "\"" << std::endl;
    return;
}

void child(int writefd, int readfd)
{
    char sendbuf[128] = "Hello parent";
    char recvbuf[128];
    read(readfd, recvbuf, 128);
    std::cout << "child process read from pipe is \"" << recvbuf << "\"" << std::endl;
    write(writefd, sendbuf, strlen(sendbuf) + 1);
    return;
}

int main()
{
    char sendbuf[128];
    char recvbuf[128];
    int fd1[2], fd2[2];
    pipe(fd1);
    pipe(fd2);
    pid_t pid = fork();
    if (pid == -1)
        return -1;
    else if (pid == 0)
    {
        close(fd1[1]);
        close(fd2[0]);
        child(fd2[1],fd1[0]);
        exit(0);
    }
    else
    {
        close(fd1[0]);
        close(fd2[1]);
        parent(fd1[1], fd2[0]);
        int status;
        wait(&status);
    }
    return 0;
}