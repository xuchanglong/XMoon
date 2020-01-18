#ifndef XMOON__INCLUDE_XMN_THREADPOOL_H_
#define XMOON__INCLUDE_XMN_THREADPOOL_H_

#include "base/noncopyable.h"

#include <pthread.h>
#include <string.h>
#include <vector>
#include <atomic>
#include <list>

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
     * @ret  0   操作成功。
     * @author  xuchanglong
     * @time    2019-09-04
    */
    int Create(const size_t &kThreadCount);

    /**
     * @funtion 释放线程池中所有线程。
     * @paras   none 。
     * @ret  0   操作成功。
     * @author  xuchanglong
     * @time    2019-09-04
    */
    int Destroy();

    /**
     * @function    唤醒一个线程开始执行任务。
     * @paras   none 。
     * @ret  0   操作成功
     * @author  xuchanglong
     * @time    2019-09-05
    */
    int Call();

    /**
     * @function    将接收到的数据压入消息链表中。
     * @paras   pdata   接收到的数据。
     * @ret  none 。
     * @author  xuchanglong
     * @time    2019-09-01
    */
    int PutInRecvMsgList_Signal(char *pdata);

    /**
     * @function    获取消息的数量
     * @paras   none 。
     * @ret  消息的数量
     * @author  xuchanglong
     * @time    2019-09-12
    */
    size_t RecvMsgListSize();

private:
    /**
     * @function    线程的执行入口函数。
     * @paras   pthreaddata 保存当前线程信息的内存。
     * @ret  nullptr 。
     * @author  xuchanglong
     * @time 2019-09-07
    */
    static void *ThreadFunc(void *pthreaddata);

    /**
     * @function    从消息链表中获取消息。
     * @paras   none 。
     * @ret  非0 获取消息成功。
     *                  nullptr 获取消息失败。
     * @author  xuchanglong
     * @time    2019-09-06
    */
    char *PutOutRecvMsgList();

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
    std::atomic<size_t> threadrunningcount_;

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

    /**
     * 存放接收的数据的消息链表。
    */
    std::list<char *> recvmsglist_;
};

#endif