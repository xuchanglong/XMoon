/*****************************************************************************************
 * @function    使用读写锁实现4个线程读写一段内存的实例。
 * @notice      在任意时刻，如果有一个线程在写数据，将阻塞所有其他线程的任何操作。
 * @author      xuchanglong
 * @time        2019-09-17  
 *****************************************************************************************/
#include <pthread.h>
#include <iostream>
#include <string.h>
#include <unistd.h>

void *thread_func_read_one(void *arg);
void *thread_func_read_two(void *arg);
void *thread_func_write_one(void *arg);
void *thread_func_write_two(void *arg);

using threadfunc = void *(*)(void *arg);

static pthread_rwlock_t rwlock;
static char workarea[1024];
static bool isexitall = false;

static const threadfunc threadfuncsum[] =
    {
        thread_func_read_one,
        thread_func_read_two,
        thread_func_write_one,
        thread_func_write_two,
};

#define threadsum (sizeof(threadfuncsum) / sizeof(threadfunc))

int main()
{
    int res = 0;
    pthread_t thread_t_sum[threadsum];

    /**
     * （1）初始化读写锁。
    */
    res = pthread_rwlock_init(&rwlock, nullptr);
    if (res != 0)
    {
        std::cout << "rwlock initialization failed." << std::endl;
        return 1;
    }

    /**
     * （2）开启所有线程。
    */
    for (size_t i = 0; i < threadsum; i++)
    {
        res = pthread_create(&thread_t_sum[i], nullptr, threadfuncsum[i], nullptr);
        if (res != 0)
        {
            std::cout << "thread creation failed." << std::endl;
            return 2;
        }
    }

    /**
     * （3）等待所有线程的退出。
    */
    for (size_t i = 0; i < threadsum; i++)
    {
        res = pthread_join(thread_t_sum[i], nullptr);
        if (res != 0)
        {
            std::cout << "create join failed." << std::endl;
            return 3;
        }
    }

    /**
     * （4）销毁读写锁。
    */
    pthread_rwlock_destroy(&rwlock);
    return 0;
}

void *thread_func_read_one(void *arg)
{
    std::cout << "read thread one start run." << std::endl;
    while (strncmp("end", workarea, 3) != 0)
    {
        pthread_rwlock_rdlock(&rwlock);
        std::cout << "read thread one get rwlock.";
        std::cout << "workarea is :" << workarea << std::endl;
        pthread_rwlock_unlock(&rwlock);
        sleep(2);
    }
    isexitall = true;
    return nullptr;
}

void *thread_func_read_two(void *arg)
{
    std::cout << "read thread two start run." << std::endl;
    while (strncmp("end", workarea, 3) != 0)
    {
        pthread_rwlock_rdlock(&rwlock);
        std::cout << "read thread two get rwlock.";
        std::cout << "workarea is :" << workarea << std::endl;
        pthread_rwlock_unlock(&rwlock);
        sleep(5);
    }
    isexitall = true;
    return nullptr;
}

void *thread_func_write_one(void *arg)
{
    std::cout << "write thread one start run." << std::endl;
    while (!isexitall)
    {
        pthread_rwlock_wrlock(&rwlock);
        std::cout << "write thread one get rwlock.";
        std::cout << "please input data:";
        std::cin >> workarea;
        pthread_rwlock_unlock(&rwlock);
        sleep(15);
    }

    return nullptr;
}

void *thread_func_write_two(void *arg)
{
    std::cout << "write thread two start run." << std::endl;
    while (!isexitall)
    {
        pthread_rwlock_wrlock(&rwlock);
        std::cout << "write thread two get rwlock.";
        std::cout << "please input data:";
        std::cin >> workarea;
        pthread_rwlock_unlock(&rwlock);
        sleep(20);
    }

    return nullptr;
}