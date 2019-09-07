#ifndef XMOON__INCLUDE_XMN_THREADPOOL_H_
#define XMOON__INCLUDE_XMN_THREADPOOL_H_

#include "base/noncopyable.h"

#include <pthread.h>
#include <string.h>
#include <vector>
#include <atomic>

class XMNThreadPool : public NonCopyable
{
private:
    /**
     * 保存单个线程的信息。
    */
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
        /**
         * 该线程所在的线程池的首地址。
        */
        XMNThreadPool *pthreadpool_;
        /**
         * 该线程是否在运行。
        */
        bool isrunning_;
        /**
         * 该线程的描述符。
        */
        pthread_t threadhandle_;
    };

public:
    XMNThreadPool();
    ~XMNThreadPool();

public:
    /**
     * @function    创建线程池。
     * @paras   kThreadCount 线程池中线程的数量。
     * @return  0   操作成功。
     * @author  xuchanglong
     * @time    2019-09-04
    */
    int Create(const size_t &kThreadCount);

    /**
     * @funtion 释放线程池中所有线程。
     * @paras   none 。
     * @return  0   操作成功。
     * @author  xuchanglong
     * @time    2019-09-04
    */
    int Destroy();

    /**
     * @function    唤醒一个线程开始执行任务。
     * @paras   none 。
     * @return  0   操作成功
     * @author  xuchanglong
     * @time    2019-09-05
    */
    int Call();

private:
    /**
     * @function    线程的执行入口函数。
     * @paras   pthreaddata 保存当前线程信息的内存。
     * @return  nullptr 。
     * @author  xuchanglong
     * @time 2019-09-07
    */
    static void *ThreadFunc(void *pthreaddata);

private:
    /**
     * 线程池中线程的数量。
    */
    size_t threadpoolsize_;

    /**
     * 保持线程池中每个线程的信息。
    */
    std::vector<ThreadInfo *> vthreadinfo_;

    /**
     * 线程是否退出的标识。
    */
    static bool isquit_;

    /**
     * 线程池中正在运行的线程的数量。
    */
    std::atomic<size_t> threadrunningcount;

    /**
     * 线程同步互斥量。
    */
    static pthread_mutex_t thread_mutex_;

    /**
     * 线程同步条件。
    */
    static pthread_cond_t thread_cond_;

    /**
     * 记录上次线程不够用时发生的时间。
    */
    time_t lasttime_;
};

#endif