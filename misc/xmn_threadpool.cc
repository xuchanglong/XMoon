#include "xmn_threadpool.h"
#include "xmn_func.h"
#include "xmn_global.h"
#include "xmn_memory.h"
#include "xmn_lockmutex.hpp"

#include <errno.h>
#include <unistd.h>

XMNThreadPool::XMNThreadPool()
{
    threadpoolsize_ = 0;
    threadrunningcount_ = 0;
    allthreadswork_lasttime_ = 0;
    queue_recvdata_count_ = 0;

    thread_mutex_ = PTHREAD_MUTEX_INITIALIZER;
    recvdata_queue_mutex_ = PTHREAD_MUTEX_INITIALIZER;
    queue_thread_cond_mutex = PTHREAD_MUTEX_INITIALIZER;

    isquit_ = false;
}

XMNThreadPool::~XMNThreadPool()
{
    while (queue_thread_cond_.size())
    {
        pthread_cond_t *ptmp = queue_thread_cond_.front();
        queue_thread_cond_.pop();
        delete ptmp;
    }
    std::queue<pthread_cond_t *>().swap(queue_thread_cond_);
    std::queue<char *>().swap(recvdata_queue_);
}

int XMNThreadPool::Create(const size_t &kThreadCount)
{
    int r = 0;
    threadpoolsize_ = kThreadCount;
    for (size_t i = 0; i < threadpoolsize_; i++)
    {
        ThreadInfo *pthreadinfoitem = new ThreadInfo(this);
        pthread_cond_t *pcond = new pthread_cond_t();
        pthread_cond_init(pcond, nullptr);
        pthreadinfoitem->SetCond(pcond);
        r = pthread_create(&pthreadinfoitem->threadhandle_, nullptr, ThreadFunc, (void *)pthreadinfoitem);
        if (r != 0)
        {
            XMNLogStdErr(errno, "XMNThreadPool::Create()中创建线程 %d 失败，返回的错误码为 %d 。", i, errno);
            return -1;
        }
        vthreadinfo_.push_back(pthreadinfoitem);
    }
lbcheck:
    for (const auto &x : vthreadinfo_)
    {
        if (!x->isrunning_)
        {
            usleep(100 * 1000);
            goto lbcheck;
        }
    }

    return 0;
}

void *XMNThreadPool::ThreadFunc(void *pthreaddata)
{
    if (pthreaddata == nullptr)
        return nullptr;
    int r = 0;
    ThreadInfo *pthreadinfo = (ThreadInfo *)pthreaddata;
    XMNThreadPool *pthreadpool = pthreadinfo->pthreadpool_;
    pthread_cond_t *pcond = pthreadinfo->GetCond();

    char *pmsg = nullptr;

    const pthread_t pid = pthread_self();
    while (!pthreadpool->isquit_)
    {
        r = pthread_mutex_lock(&pthreadpool->thread_mutex_);
        if (r != 0)
            XMNLogStdErr(r, "XMNThreadPool::ThreadFunc 中 pthread_mutex_lock 执行失败。");

        while ((!pthreadpool->isquit_) && (pmsg = pthreadpool->PutOutRecvDataQueue()) == nullptr)
        {
            pthreadinfo->isrunning_ = true;
            pthread_mutex_lock(&pthreadpool->queue_thread_cond_mutex);
            pthreadpool->queue_thread_cond_.push(pcond);
            pthread_mutex_unlock(&pthreadpool->queue_thread_cond_mutex);
            pthread_cond_wait(pcond, &pthreadpool->thread_mutex_);
            pmsg = nullptr;
        }
        r = pthread_mutex_unlock(&pthreadpool->thread_mutex_);
        if (r != 0)
            XMNLogStdErr(r, "XMNThreadPool::ThreadFunc 中 pthread_mutex_unlock 执行失败。");
        ++pthreadpool->threadrunningcount_;
        g_socket.ThreadRecvProcFunc(pmsg);
        --pthreadpool->threadrunningcount_;
    }
    pthread_cond_destroy(pcond);
    return 0;
}

int XMNThreadPool::Destroy()
{
    int r = 0;
    if (isquit_)
        return -1;
    isquit_ = true;
    while (queue_thread_cond_.size())
    {
        pthread_mutex_lock(&queue_thread_cond_mutex);
        pthread_cond_t *pcond = queue_thread_cond_.front();
        queue_thread_cond_.pop();
        pthread_mutex_unlock(&queue_thread_cond_mutex);

        r = pthread_cond_signal(pcond);
        if (r != 0)
            XMNLogStdErr(r, "XMNThreadPool::Destroy() 中 pthread_cond_signal 执行失败。");
    }
    for (const auto &x : vthreadinfo_)
        pthread_join(x->threadhandle_, nullptr);
    vthreadinfo_.clear();
    std::vector<ThreadInfo *>().swap(vthreadinfo_);
    return 0;
}

int XMNThreadPool::Call()
{
    pthread_cond_t *pcond = nullptr;
    while (true)
    {
        pthread_mutex_lock(&queue_thread_cond_mutex);
        pcond = queue_thread_cond_.front();
        if (!pcond)
        {
            pthread_mutex_unlock(&queue_thread_cond_mutex);
            usleep(10);
            continue;
        }
        queue_thread_cond_.pop();
        pthread_mutex_unlock(&queue_thread_cond_mutex);
        break;
    }

    int r = pthread_cond_signal(pcond);
    if (r != 0)
        XMNLogStdErr(r, "XMNThreadPool::Call 中 pthread_cond_signal 执行失败。");
    if (threadpoolsize_ == threadrunningcount_)
    {
        time_t currtime = time(nullptr);
        if (currtime - allthreadswork_lasttime_ > 10)
        {
            allthreadswork_lasttime_ = currtime;
            XMNLogStdErr(0, "线程池满负荷运转，可考虑扩容线程池。");
        }
    }

    return 0;
}

int XMNThreadPool::PutInRecvDataQueue_Signal(char *data)
{
    int r = 0;
    r = pthread_mutex_lock(&recvdata_queue_mutex_);
    if (r != 0)
        XMNLogStdErr(r, "XMNThreadPool::PutInRecvDataQueue_Signal 中的 pthread_mutex_lock 执行失败。");

    recvdata_queue_.push(data);
    queue_recvdata_count_++;

    r = pthread_mutex_unlock(&recvdata_queue_mutex_);
    if (r != 0)
        XMNLogStdErr(r, "XMNThreadPool::PutInRecvDataQueue_Signal 中的 pthread_mutex_unlock 执行失败。");
    Call();
    return 0;
}

char *XMNThreadPool::PutOutRecvDataQueue()
{
    XMNLockMutex lockmutex_recvdata(&recvdata_queue_mutex_);
    char *pbuf = nullptr;
    if (queue_recvdata_count_ > 0)
    {
        pbuf = recvdata_queue_.front();
        recvdata_queue_.pop();
        queue_recvdata_count_--;
    }

    return pbuf;
}

size_t XMNThreadPool::RecvDataQueueSize()
{
    return queue_recvdata_count_;
}