#include <iostream>
#include <semaphore.h>
#include <sys/shm.h>
#include <fcntl.h>
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
    psem = sem_open(kSemName, 0);
    if (psem == SEM_FAILED)
    {
        std::cout << "Failed to get sem，error no = " << errno << std::endl;
        sem_close(psem);
        return -1;
    }
    // 获取共享内存。
    shmid = shmget(key, kShmSize, 0666);
    if (shmid < 0)
    {
        std::cout << "Failed to get shm，error no = " << errno << std::endl;
        sem_close(psem);
        return -2;
    }

    pshm = (char *)shmat(shmid, nullptr, !SHM_RDONLY);
    s = pshm;
    for (s = pshm; *s != '\0'; s++)
    {
        sem_post(psem);
        std::cout << *s;
        sem_wait(psem);
    }
    std::cout << std::endl;
    *s = '*';
    sem_close(psem);
    shmctl(shmid, IPC_RMID, 0);

    return 0;
}