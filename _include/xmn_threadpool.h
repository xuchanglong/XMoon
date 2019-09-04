#ifndef XMOON__INCLUDE_XMN_THREADPOOL_H_
#define XMOON__INCLUDE_XMN_THREADPOOL_H_

#include "base/noncopyable.h"
#include <pthread.h>

class XMNThreadPool : public NonCopyable
{
private:
    /**
     * 保存单个线程的信息。
    */
    class ThreadInfo
    {
    public:
        ThreadInfo(XMNThreadPool *ppool) : pthreadpool_(ppool)
        {
            isrunning_ = false;
            memset(&threadhandle_, 0, sizeof(pthread_t));
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
     * @paras   threadcount 线程池中线程的数量。
     * @return  0   操作成功。
     * @author  xuchanglong
     * @time    2019-09-04
    */
    int Create(const int &threadcount);

    /**
     * @funtion 释放线程池中所有线程。
     * @paras   none 。
     * @return  0   操作成功。
     * @time    2019-09-04
    */
    int Delete();
};

#endif