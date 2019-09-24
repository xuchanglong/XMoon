#include "xmn_threadpool.h"
#include "xmn_func.h"
#include "xmn_global.h"
#include "xmn_memory.h"

#include <errno.h>
#include <unistd.h>

bool XMNThreadPool::isquit_ = false;
pthread_mutex_t XMNThreadPool::thread_mutex_ = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t XMNThreadPool::thread_cond_ = PTHREAD_COND_INITIALIZER;

XMNThreadPool::XMNThreadPool()
{
    threadpoolsize_ = 0;
    threadrunningcount_ = 0;
    lasttime_ = 0;
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
     * （1）创建指定数量的线程。
    */
    for (size_t i = 0; i < threadpoolsize_; i++)
    {
        ThreadInfo *pthreadinfoitem = new ThreadInfo(this);

        /**
         * 线程创建完毕之后，该线程就会开始被系统调用，即：ThreadFunc 开始运行。
        */
        r = pthread_create(&pthreadinfoitem->threadhandle_, nullptr, ThreadFunc, (void *)pthreadinfoitem);
        if (r != 0)
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
    if (pthreaddata == nullptr)
    {
        return nullptr;
    }
    int r = 0;
    ThreadInfo *pthreadinfo = (ThreadInfo *)pthreaddata;
    XMNThreadPool *pthreadpool = pthreadinfo->pthreadpool_;
    XMNMemory *pmemory = SingletonBase<XMNMemory>::GetInstance();
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

        while ((!isquit_) && ((pmsg = pthreadpool->PutOutRecvMsgList()) == nullptr))
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
        ++pthreadpool->threadrunningcount_;

        /**
         * 开始业务处理。
        */
        //xmn_log_stderr(0, "业务逻辑开始执行，pid = %d", pid);
        //sleep(5);
        g_socket.ThreadRecvProcFunc(pmsg);

        pmemory->FreeMemory((void *)pmsg);
        /**
         * 业务处理结束。
        */
        //xmn_log_stderr(0, "业务逻辑执行结束，pid = %d", pid);

        /**
         * 正在运行的线程数 - 1 。
        */
        --pthreadpool->threadrunningcount_;
    }
    return nullptr;
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
        xmn_log_stderr(r, "XMNThreadPool::Destroy() 中 pthread_cond_signal 执行失败。");
    }

    /**
     * （2）等待线程池中的线程都退出。
    */
    std::vector<ThreadInfo *>::iterator it;
    for (it = vthreadinfo_.begin(); it != vthreadinfo_.end(); ++it)
    {
        /**
         * @function    1、等待指定的线程退出。
         *                           2、释放退出的线程的系统资源。
        */
        pthread_join((*it)->threadhandle_, nullptr);
    }

    /**
     * （3）销毁条件变量和互斥量、释放存储线程池中各个线程信息的内存。
    */
    pthread_cond_destroy(&thread_cond_);
    pthread_mutex_destroy(&thread_mutex_);

    for (it = vthreadinfo_.begin(); it != vthreadinfo_.end(); ++it)
    {
        delete (*it);
        (*it) = nullptr;
    }
    vthreadinfo_.clear();

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
        xmn_log_stderr(r, "XMNThreadPool::Call 中 pthread_cond_signal 执行失败。");
    }

    /**
     * （2）若线程池中线程满负荷运作，则报警显示。
    */
    if (threadpoolsize_ == threadrunningcount_)
    {
        time_t currtime = time(nullptr);
        if (currtime - lasttime_ > 10)
        {
            lasttime_ = currtime;
            xmn_log_stderr(0, "线程池满负荷运转，可考虑扩容线程池。");
        }
    }

    return 0;
}

int XMNThreadPool::PutInRecvMsgList_Signal(char *pdata)
{
    int r = 0;
    if (pdata == nullptr)
    {
        return -1;
    }
    //xmn_log_stderr(errno, "已接收到完整数据包。");
    /**
     * （1）向消息链表中压入 client 发来的数据。
    */
    r = pthread_mutex_lock(&thread_mutex_);
    if (r != 0)
    {
        xmn_log_stderr(r, "XMNThreadPool::inMsgRecvQueueAndSignal 中的 pthread_mutex_lock 执行失败。");
    }

    recvmsglist_.push_back(pdata);

    r = pthread_mutex_unlock(&thread_mutex_);
    if (r != 0)
    {
        xmn_log_stderr(r, "XMNThreadPool::inMsgRecvQueueAndSignal 中的 pthread_mutex_unlock 执行失败。");
    }
    /**
     * （2）激发线程池中的一个线程从消息链表中取走消息并处理。
    */
    Call();
    return 0;
}

char *XMNThreadPool::PutOutRecvMsgList()
{
    if (recvmsglist_.empty())
    {
        return nullptr;
    }

    char *pbuf = recvmsglist_.front();
    recvmsglist_.pop_front();

    return pbuf;
}

size_t XMNThreadPool::RecvMsgListSize()
{
    return recvmsglist_.size();
}