#ifndef XMOON__INCLUDE_XMN_THREADPOOL_H_
#define XMOON__INCLUDE_XMN_THREADPOOL_H_

#include "base/noncopyable.h"
#include <pthread.h>
#include <string.h>
#include <vector>

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

    private:
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
    int Create(const int &kThreadCount);

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
     * @paras   kMsgCount    消息队列中消息的数量。
     * @return  0   操作成功
     * @author  xuchanglong
     * @time    2019-09-05
    */
    int Call(const size_t &kMsgCount);

private:
    /**
     * @function    新创建的线程的入口函数。
     * TODO：后续补充。
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
};

#endif