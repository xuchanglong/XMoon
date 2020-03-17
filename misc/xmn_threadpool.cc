#include "xmn_threadpool.h"
#include "xmn_func.h"
#include "xmn_global.h"
#include "xmn_memory.h"
#include "xmn_lockmutex.hpp"

#include <errno.h>
#include <unistd.h>

bool XMNThreadPool::isquit_ = false;
pthread_mutex_t XMNThreadPool::thread_mutex_ = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t XMNThreadPool::thread_cond_ = PTHREAD_COND_INITIALIZER;

pthread_mutex_t XMNThreadPool::recvdata_queue_mutex_ = PTHREAD_MUTEX_INITIALIZER;

XMNThreadPool::XMNThreadPool()
{
    threadpoolsize_ = 0;
    threadrunningcount_ = 0;
    allthreadswork_lasttime_ = 0;
    queue_recvdata_count_ = 0;
}

XMNThreadPool::~XMNThreadPool()
{
    pthread_mutex_destroy(&thread_mutex_);
    pthread_mutex_destroy(&recvdata_queue_mutex_);
    pthread_cond_destroy(&thread_cond_);
}

int XMNThreadPool::Create(const size_t &kThreadCount)
{
    int r = 0;
    threadpoolsize_ = kThreadCount;

    /**
     * （1）创建指定数量的线程。
    */
    for (size_t i = 0; i < threadpoolsize_; i++)
    {
        ThreadInfo *pthreadinfoitem = new ThreadInfo(this);
        r = pthread_create(&pthreadinfoitem->threadhandle_, nullptr, ThreadFunc, (void *)pthreadinfoitem);
        if (r != 0)
        {
            XMNLogStdErr(errno, "XMNThreadPool::Create()中创建线程 %d 失败，返回的错误码为 %d 。", i, errno);
            return -1;
        }
        vthreadinfo_.push_back(pthreadinfoitem);
    }

    /**
     * （2）等待所有的线程都卡在 pthread_cond_wait() 。
    */
lbcheck:
    for (const auto &x : vthreadinfo_)
    {
        if (!x->isrunning_)
        {
            /**
             * 如果尚有未准备就绪的线程，则延时 100ms 。
            */
            usleep(100 * 1000);
            goto lbcheck;
        }
    }

    return 0;
}

void *XMNThreadPool::ThreadFunc(void *pthreaddata)
{
    if (pthreaddata == nullptr)
    {
        return nullptr;
    }
    int r = 0;
    ThreadInfo *pthreadinfo = (ThreadInfo *)pthreaddata;
    XMNThreadPool *pthreadpool = pthreadinfo->pthreadpool_;
    char *pmsg = nullptr;

    const pthread_t pid = pthread_self();

    /**
     * 从消息链表中取出数据。
    */
    while (true)
    {
        r = pthread_mutex_lock(&thread_mutex_);
        if (r != 0)
        {
            XMNLogStdErr(r, "XMNThreadPool::ThreadFunc 中 pthread_mutex_lock 执行失败。");
        }

        while ((!isquit_) && ((pmsg = pthreadpool->PutOutRecvDataQueue()) == nullptr))
        {
            /**
             * 运行到这里，说明线程没有接收到退出命令且也没有从消息链表中拿到了消息。
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
         * 运行到这里，说明线程从消息链表中取出了数据或者线程要退出，即：isquit_ == true 。
        */
        /**
         * 解锁。
        */
        r = pthread_mutex_unlock(&thread_mutex_);
        if (r != 0)
        {
            XMNLogStdErr(r, "XMNThreadPool::ThreadFunc 中 pthread_mutex_unlock 执行失败。");
        }

        /**
         * 正在运行的线程数 + 1 。
        */
        ++pthreadpool->threadrunningcount_;

        /**
         * 开始业务处理。
        */
        //XMNLogStdErr(0, "业务逻辑开始执行，pid = %d", pid);
        //sleep(5);
        g_socket.ThreadRecvProcFunc(pmsg);

        //pmemory->FreeMemory((void *)pmsg);
        /**
         * 业务处理结束。
        */
        //XMNLogStdErr(0, "业务逻辑执行结束，pid = %d", pid);

        /**
         * 正在运行的线程数 - 1 。
        */
        --pthreadpool->threadrunningcount_;
    }
    return 0;
}

int XMNThreadPool::Destroy()
{
    int r = 0;
    /**
     * （1）置位线程退出标志并向线程池中所有线程发送消息。
    */
    if (isquit_)
    {
        return -1;
    }
    isquit_ = true;

    r = pthread_cond_broadcast(&thread_cond_);
    if (r != 0)
    {
        XMNLogStdErr(r, "XMNThreadPool::Destroy() 中 pthread_cond_signal 执行失败。");
    }

    /**
     * （2）等待线程池中的线程都退出。
    */
    for (const auto &x : vthreadinfo_)
    {
        /**
         * @function    1、等待指定的线程退出。
         *                           2、释放退出的线程的系统资源。
        */
        pthread_join(x->threadhandle_, nullptr);
    }

    /**
     * （3）销毁条件变量和互斥量、释放存储线程池中各个线程信息的内存。
    */
    pthread_cond_destroy(&thread_cond_);
    pthread_mutex_destroy(&thread_mutex_);

    vthreadinfo_.clear();
    std::vector<ThreadInfo *>().swap(vthreadinfo_);
    return 0;
}

int XMNThreadPool::Call()
{
    /**
     * （1）向线程池中的线程发生一个消息，注意仅仅对其中优先级最高的线程发送。
     * 使该线程从 pthread_cond_wait 函数跳出。
    */
    int r = pthread_cond_signal(&thread_cond_);
    if (r != 0)
    {
        XMNLogStdErr(r, "XMNThreadPool::Call 中 pthread_cond_signal 执行失败。");
    }

    /**
     * （2）若线程池中线程满负荷运作，则报警显示。
    */
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
    /**
     * （1）向消息队列中压入 client 发来的数据。
    */
    r = pthread_mutex_lock(&recvdata_queue_mutex_);
    if (r != 0)
    {
        XMNLogStdErr(r, "XMNThreadPool::PutInRecvDataQueue_Signal 中的 pthread_mutex_lock 执行失败。");
    }

    recvdata_queue_.push(data);
    queue_recvdata_count_++;

    r = pthread_mutex_unlock(&recvdata_queue_mutex_);
    if (r != 0)
    {
        XMNLogStdErr(r, "XMNThreadPool::PutInRecvDataQueue_Signal 中的 pthread_mutex_unlock 执行失败。");
    }
    /**
     * （2）激发线程池中的一个线程从消息链表中取走消息并处理。
    */
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