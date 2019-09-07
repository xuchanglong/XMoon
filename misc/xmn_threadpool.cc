#include "xmn_threadpool.h"
#include "xmn_func.h"
#include "xmn_global.h"

#include <errno.h>
#include <unistd.h>

bool XMNThreadPool::isquit_ = false;
pthread_mutex_t XMNThreadPool::thread_mutex_ = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t XMNThreadPool::thread_cond_ = PTHREAD_COND_INITIALIZER;

XMNThreadPool::XMNThreadPool()
{
    threadpoolsize_ = 0;
    threadrunningcount = 0;
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
        /**
         * 线程创建完毕之后，该线程就会开始被系统调用，即：ThreadFunc 开始运行。
        */
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
    std::vector<ThreadInfo *>::iterator it;
lbcheck:
    for (it = vthreadinfo_.begin(); it != vthreadinfo_.end(); ++it)
    {
        if (!(*it)->isrunning_)
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
    int r = 0;
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
        r = pthread_mutex_lock(&thread_mutex_);
        if (r != 0)
        {
            xmn_log_stderr(r, "XMNThreadPool::ThreadFunc 中 pthread_mutex_lock 执行失败。");
        }

        while ((!isquit_) && ((pmsg = g_socket.PutOutRecvMsgList()) == nullptr))
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
            xmn_log_stderr(r, "XMNThreadPool::ThreadFunc 中 pthread_mutex_unlock 执行失败。");
        }

        /**
         * 正在运行的线程数 + 1 。
        */
        ++pthreadpool->threadrunningcount;

        /**
         * 开始业务处理。
        */
        xmn_log_stderr(0, "业务逻辑开始执行，pid = %d", pid);
        sleep(5);
        /**
         * 业务处理结束。
        */
        xmn_log_stderr(0, "业务逻辑执行结束，pid = %d", pid);

        /**
         * 正在运行的线程数 - 1 。
        */
        --pthreadpool->threadrunningcount;
    }
    return nullptr;
}

int XMNThreadPool::Destroy()
{
    return 0;
}

int XMNThreadPool::Call()
{
    return 0;
}