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
    // 静态初始化的锁无需销毁。
    //pthread_mutex_destroy(&thread_mutex_);
    //pthread_mutex_destroy(&recvdata_queue_mutex_);
    //pthread_mutex_destroy(&queue_thread_cond_mutex);
    while (queue_thread_cond_.size())
    {
        pthread_cond_t *ptmp = queue_thread_cond_.front();
        queue_thread_cond_.pop();
        delete ptmp;
    }
    std::queue<pthread_cond_t *>().swap(queue_thread_cond_);

    // 由于在连接回收线程中，会检测线程池的 isquit 变量。
    // 如果该变量为 true，则可以释放接收数据所占的内存，
    // 故，此处不用再释放内存了。
    
    // while (recvdata_queue_.size())
    // {
    //     char *ptmp = recvdata_queue_.front();
    //     recvdata_queue_.pop();
    //     delete ptmp;
    // }
    std::queue<char *>().swap(recvdata_queue_);
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
    pthread_cond_t *pcond = pthreadinfo->GetCond();

    char *pmsg = nullptr;

    const pthread_t pid = pthread_self();

    /**
     * 从消息链表中取出数据。
    */
    while (!pthreadpool->isquit_)
    {
        r = pthread_mutex_lock(&pthreadpool->thread_mutex_);
        if (r != 0)
        {
            XMNLogStdErr(r, "XMNThreadPool::ThreadFunc 中 pthread_mutex_lock 执行失败。");
        }

        while ((!pthreadpool->isquit_) && (pmsg = pthreadpool->PutOutRecvDataQueue()) == nullptr)
        {
            /**
             * 运行到这里，说明线程没有接收到退出命令且也没有从消息链表中拿到了消息。
            */
            /**
             * 标记该线程已经开始运行。
            */
            pthreadinfo->isrunning_ = true;
            /**
             * 将该线程的 cond 压入等待队列中。
            */
            pthread_mutex_lock(&pthreadpool->queue_thread_cond_mutex);
            pthreadpool->queue_thread_cond_.push(pcond);
            //XMNLogInfo(6, 0, ("当前压入队列中的线程 pid = " + std::to_string(pid)).c_str());
            pthread_mutex_unlock(&pthreadpool->queue_thread_cond_mutex);
            /**
             * 进入该函数时，解锁。
             * 走出该函数时，加锁。
            */
            pthread_cond_wait(pcond, &pthreadpool->thread_mutex_);
            /**
             * 保证每次线程运行时都能对 pmsg 进行初始化。
            */
            pmsg = nullptr;
        }

        /**
         * 运行到这里，说明线程从消息链表中取出了数据或者线程要退出，即：isquit_ == true 。
        */
        //XMNLogInfo(6, 0, ("当前线程 pid = " + std::to_string(pid)).c_str());
        /**
         * 解锁。
        */
        r = pthread_mutex_unlock(&pthreadpool->thread_mutex_);
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
        g_socket.ThreadRecvProcFunc(pmsg);

        /**
         * 业务处理结束。
        */
        /**
         * 正在运行的线程数 - 1 。
        */
        --pthreadpool->threadrunningcount_;
    }
    pthread_cond_destroy(pcond);
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
    /**
     * 让每一个线程安全退出。
    */
    while (queue_thread_cond_.size())
    {
        pthread_mutex_lock(&queue_thread_cond_mutex);
        pthread_cond_t *pcond = queue_thread_cond_.front();
        queue_thread_cond_.pop();
        pthread_mutex_unlock(&queue_thread_cond_mutex);

        r = pthread_cond_signal(pcond);
        if (r != 0)
        {
            XMNLogStdErr(r, "XMNThreadPool::Destroy() 中 pthread_cond_signal 执行失败。");
        }
    }

    /**
     * （2）等待线程池中的线程都退出。
    */
    for (const auto &x : vthreadinfo_)
    {
        /**
         * @function    1、等待指定的线程退出。
         *              2、释放退出的线程的系统资源。
        */
        pthread_join(x->threadhandle_, nullptr);
    }

    /**
     * （3）销毁条件变量和互斥量、释放存储线程池中各个线程信息的内存。
    */
    //pthread_mutex_destroy(&thread_mutex_);

    vthreadinfo_.clear();
    std::vector<ThreadInfo *>().swap(vthreadinfo_);
    return 0;
}

int XMNThreadPool::Call()
{
    /**
     * 按照线程等待队列中的顺序依次调用线程。
     * 若无线程可掉，则休息 10us ，再次调用尝试。
    */
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