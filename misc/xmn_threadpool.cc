#include "xmn_threadpool.h"
#include "xmn_func.h"
#include <errno.h>
#include "xmn_global.h"
#include <unistd.h>

bool XMNThreadPool::isquit_ = false;

XMNThreadPool::XMNThreadPool()
{
    threadpoolsize_ = 0;
}

XMNThreadPool::~XMNThreadPool()
{
    ;
}

int XMNThreadPool::Create(const size_t &kThreadCount)
{
    int r = 0;
    threadpoolsize_ = kThreadCount;
    /**
     * （1）创建指定数量的线程池。
    */
    for (size_t i = 0; i < threadpoolsize_; i++)
    {
        ThreadInfo *pthreadinfoitem = new ThreadInfo(this);
        r = pthread_create(&pthreadinfoitem->threadhandle_, nullptr, ThreadFunc, (void *)pthreadinfoitem);
        if (r == -1)
        {
            xmn_log_stderr(errno, "XMNThreadPool::Create()中创建线程 %d 失败，返回的错误码为 %d 。", i, errno);
            return -1;
        }
        vthreadinfo_.push_back(pthreadinfoitem);
    }

    /**
     * （2）等待所有的线程都卡在 pthread_cond_wait() 。
    */
    /**
     * TODO:编写完 ThreadFunc 代码再完成这部分代码。
    */

    return 0;
}

void *XMNThreadPool::ThreadFunc(void *pthreaddata)
{
    if (pthreaddata == nullptr)
    {
        return nullptr;
    }
    ThreadInfo *pthreadinfo = (ThreadInfo *)pthreaddata;
    XMNThreadPool *pthreadpool = pthreadinfo->pthreadpool_;
    char *pmsg = nullptr;

    size_t pid = pthread_self();
    /**
     * 从消息链表中取出数据。
    */
    while (true)
    {
        pthread_mutex_lock(&thread_mutex_);
        while ((!isquit_) && ((pmsg = g_socket.PutOutRecvMsgList()) == nullptr))
        {
            /**
             * 运行到这里，说明线程没有接收到退出命令且从消息队列中拿到了消息。
            */
            /**
             * 标记该线程已经开始运行。
            */
            pthreadinfo->isrunning_ = true;
            /**
             * 进入该函数时，解锁。
             * 走出该函数时，加锁。
            */
            pthread_cond_wait(&thread_cond_, &thread_mutex_);
        }

        /**
         * 运行到这里，说明线程从消息链表中取出了数据或者 isquit_ == true 。
        */

        /**
         * 解锁。
        */
        int err = pthread_mutex_unlock(&thread_mutex_);

        /**
         * 正在运行的线程数 + 1 。
        */
        ++pthreadpool->threadrunningcount;

        /**
         * 开始业务处理。
        */
        sleep(5);
        /**
         * 业务处理结束。
        */
        --pthreadpool->threadrunningcount;
    }
    return nullptr;
}