/*****************************************************************************************
 * @function    使用读写锁实现4个线程读写一段内存的实例。
 * @notice      在任意时刻，如果有一个线程在写数据，将阻塞所有其他线程的任何操作。
 * @time        2019-09-17  
 * @notice
 * 1、读写锁有3种状态：
 * （1）读加锁
 * （2）写加锁
 * （3）不加锁
 * 2、读加锁
 * （1）不堵塞其他读线程。
 * （2）堵塞其他写线程。
 * （3）当堵塞了写线程时，后面的读线程也会被堵塞。这么做的目的是防止该资源被读线程长期占用。
 * 3、写加锁
 * （1）堵塞其他的读线程。
 * （2）堵塞其他的写线程。
 * 4、不加锁
 * 5、linux 下的 api
 * （1）创建和销毁
 * 初始化：pthread_rwlock_init()
 * 销毁：pthread_rwlock_destroy()
 * （2）加锁和解锁
 * 读线程加锁：pthread_rwlock_rdlock()
 * 写线程加锁：pthread_rwlock_wrlock()
 * 读/写线程解锁：pthread_rwlock_unlock()
 *****************************************************************************************/
#include <pthread.h>
#include <iostream>
#include <string.h>
#include <unistd.h>

void *thread_func_read_one(void *arg);
void *thread_func_read_two(void *arg);
void *thread_func_write_one(void *arg);
void *thread_func_write_two(void *arg);

using ThreadFunc = void *(*)(void *arg);

static pthread_rwlock_t rwlock;
static char workarea[1024];
static bool isexitall = false;

static const ThreadFunc threadfuncsum[] =
    {
        thread_func_read_one,
        thread_func_read_two,
        thread_func_write_one,
        thread_func_write_two,
};

#define threadsum (sizeof(threadfuncsum) / sizeof(ThreadFunc))

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