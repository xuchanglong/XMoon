#ifndef XMOON__INCLUDE_XMN_THREADPOOL_H_
#define XMOON__INCLUDE_XMN_THREADPOOL_H_

#include "base/noncopyable.h"

#include <pthread.h>
#include <string.h>
#include <vector>
#include <atomic>
#include <queue>
#include <memory>

class XMNThreadPool : public NonCopyable
{
private:
    class ThreadInfo : public NonCopyable
    {
    public:
        ThreadInfo() = delete;
        ThreadInfo(XMNThreadPool *ppool) : pthreadpool_(ppool)
        {
            isrunning_ = false;
            threadhandle_ = 0;
        }
        ~ThreadInfo(){};

    public:
        pthread_cond_t *GetCond()
        {
            return pcond_;
        }

        void SetCond(pthread_cond_t *pcond)
        {
            pcond_ = pcond;;
        }

    public:
        XMNThreadPool *pthreadpool_;
        bool isrunning_;
        pthread_t threadhandle_;
        pthread_cond_t *pcond_;
    };

public:
    XMNThreadPool();
    ~XMNThreadPool();

public:
    int Create(const size_t &kThreadCount);
    int Destroy();
    int Call();
    int PutInRecvDataQueue_Signal(char *data);
    size_t RecvDataQueueSize();

private:
    static void *ThreadFunc(void *pthreaddata);
    char *PutOutRecvDataQueue();

private:
    size_t threadpoolsize_;
    std::vector<ThreadInfo *> vthreadinfo_;
    bool isquit_;
    std::atomic<size_t> threadrunningcount_;
    pthread_mutex_t thread_mutex_;
    std::queue<pthread_cond_t *> queue_thread_cond_;
    pthread_mutex_t queue_thread_cond_mutex;
    time_t allthreadswork_lasttime_;
    pthread_mutex_t recvdata_queue_mutex_;
    std::queue<char *> recvdata_queue_;
    std::atomic<size_t> queue_recvdata_count_;
};

#endif