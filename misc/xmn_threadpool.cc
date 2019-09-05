#include "xmn_threadpool.h"
#include "xmn_func.h"
#include <errno.h>

XMNThreadPool::XMNThreadPool()
{
    threadpoolsize_ = 0;
}

XMNThreadPool::~XMNThreadPool()
{
    ;
}

void *XMNThreadPool::ThreadFunc(void *pthreaddata)
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