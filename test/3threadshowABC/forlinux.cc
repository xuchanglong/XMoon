#include <iostream>
#include <pthread.h>
#include <unistd.h>

using THREADFUNC = void *(*)(void *);

pthread_mutex_t g_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t g_condCA = PTHREAD_COND_INITIALIZER;
pthread_cond_t g_condAB = PTHREAD_COND_INITIALIZER;
pthread_cond_t g_condBC = PTHREAD_COND_INITIALIZER;

// 标记指定的线程是否处于等待 cond 的状态。
// true     处于等待 cond 的状态。
// false    cond 已成立或者处于阻塞状态。
bool g_flag_CA, g_flag_AB, g_flag_BC;
char g_showcount = 10;

void *PrintA(void *arg);
void *PrintB(void *arg);
void *PrintC(void *arg);

void MyCondWait(bool *pflag, pthread_cond_t *pcond, pthread_mutex_t *pmtx);
void MyCondSignal(bool *pflag, pthread_cond_t *pcond, pthread_mutex_t *pmtx);

int main()
{
    pthread_t thread_pid[3] = {0};
    THREADFUNC threadfunc[3] = {PrintA, PrintB, PrintC};
    char threadcount = sizeof(threadfunc) / sizeof(THREADFUNC);

    for (size_t i = 0; i < threadcount; i++)
    {
        if (pthread_create(thread_pid, nullptr, threadfunc[i], nullptr) != 0)
        {
            std::cout << "Failed to create thread." << std::endl;
            exit(1);
        }
    }
    // 加 sleep(1) 的作用是使三个线程在执行完 sleep(1) 之后，
    // 均堵塞在 pthread_cond_wait 。等待各自的条件的释放。
    sleep(1);

    pthread_mutex_lock(&g_mtx);
    MyCondSignal(&g_flag_CA, &g_condCA, &g_mtx);
    pthread_mutex_unlock(&g_mtx);

    for (size_t i = 0; i < threadcount; i++)
    {
        pthread_join(thread_pid[i], nullptr);
    }

    return 0;
}

void *PrintA(void *arg)
{
    char showcount = g_showcount;
    pthread_t pid= pthread_self();
    while (showcount--)
    {
        pthread_mutex_lock(&g_mtx);
        MyCondWait(&g_flag_CA, &g_condCA, &g_mtx);

        std::cout << g_showcount - showcount << "." << pid << "、";

        MyCondSignal(&g_flag_AB, &g_condAB, &g_mtx);
        // 解的锁是 pthread_cond_wait 满足条件时加的锁。
        pthread_mutex_unlock(&g_mtx);
    }
    // 防止线程 C ，在执行 MyCondSignal 函数时，由于 g_flag_CA = 0，导致程序执行在 while 循环中。
    g_flag_CA = true;
    return nullptr;
}

void *PrintB(void *arg)
{
    char showcount = g_showcount;
    pthread_t pid= pthread_self();
    while (showcount--)
    {
        pthread_mutex_lock(&g_mtx);
        MyCondWait(&g_flag_AB, &g_condAB, &g_mtx);

        std::cout << pid << "、";

        MyCondSignal(&g_flag_BC, &g_condBC, &g_mtx);
        pthread_mutex_unlock(&g_mtx);
    }

    return nullptr;
}

void *PrintC(void *arg)
{
    char showcount = g_showcount;
    pthread_t pid= pthread_self();
    while (showcount--)
    {
        pthread_mutex_lock(&g_mtx);
        MyCondWait(&g_flag_BC, &g_condBC, &g_mtx);

        std::cout << pid << std::endl;

        MyCondSignal(&g_flag_CA, &g_condCA, &g_mtx);
        pthread_mutex_unlock(&g_mtx);
    }

    return nullptr;
}

// 标记并使指定的线程处于等待 cond 满足的状态。
void MyCondWait(bool *pflag, pthread_cond_t *pcond, pthread_mutex_t *pmtx)
{
    *pflag = true;
    pthread_cond_wait(pcond, pmtx);
    *pflag = false;
    return;
}

// 向指定的线程发出条件已满足的通知。
void MyCondSignal(bool *pflag, pthread_cond_t *pcond, pthread_mutex_t *pmtx)
{
    while (!*pflag)
    {
        // 执行到这里，说明该线程阻塞在 while 循环开头的 pthread_mutex_lock 函数位置。
        // 需要调用 pthread_mutex_unlock() 函数将其解锁并执行到 pthread_cond_wait() 函数，使之处于等
        // 待 cond 的状态。
        pthread_mutex_unlock(pmtx);
        usleep(50);
        pthread_mutex_lock(pmtx);
    }
    // 执行到这里，说明指定的线程已经处于等待 cond 满足的状态。
    pthread_cond_signal(pcond);
    return;
}
