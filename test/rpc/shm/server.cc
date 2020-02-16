/**
 * 共享内存和信号量的测试代码。
 * 
 * POSIX 信号量：UNIX 环境高级编程 P466 。
 * 共享内存：UNIX 环境高级编程 P459 。
*/
#include <iostream>
#include <semaphore.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

const char *kSemName = "XCL";
const size_t kShmSize = 27;

int main()
{
    sem_t *psem = nullptr;
    key_t key = 1000;
    int shmid = 0;
    char *pshm = nullptr;
    char *s = nullptr;

    // 创建并初始化 semaphore 。
    psem = sem_open(kSemName, O_CREAT, 0644, 1);
    if (psem == SEM_FAILED)
    {
        std::cout << "Failed to create sem，error no = " << errno << std::endl;
        sem_unlink(kSemName);
        return -1;
    }

    // 创建并初始化共享内存。
    shmid = shmget(key, kShmSize, IPC_CREAT | 0666);
    if (shmid < 0)
    {
        std::cout << "Failed to create shm，error no = " << errno << std::endl;
        sem_unlink(kSemName);
        return -2;
    }
    pshm = (char *)shmat(shmid, nullptr, 0);
    if (pshm < 0)
    {
        std::cout << "Failed to get shm pointer，error no = " << errno << std::endl;
        sem_unlink(kSemName);
        return -3;
    }
    s = pshm;
    for (char i = 'A'; i <= 'Z'; i++)
    {
        sem_wait(psem);
        *s++ = i;
        sem_post(psem);
    }
    while (*s != '*')
    {
        sleep(1);
    }

    sem_close(psem);
    sem_unlink(kSemName);
    shmctl(shmid, IPC_RMID, 0);

    return 0;
}