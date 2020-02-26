/*****************************************************************************************
 * 
 *  @function 用于自动释放互斥量，防止函数结束之后忘记释放。
 *  @time   2019-09-05
 * 
 *****************************************************************************************/
#ifndef XMOON__INCLUDE_XMN_LOCKMUTEX_HPP_
#define XMOON__INCLUDE_XMN_LOCKMUTEX_HPP_

#include <base/noncopyable.h>
#include <pthread.h>

class XMNLockMutex : NonCopyable
{
public:
    XMNLockMutex() = delete;
    XMNLockMutex(pthread_mutex_t *pmutex) : pmutex_(pmutex)
    {
        pthread_mutex_lock(pmutex_);
    }
    ~XMNLockMutex()
    {
        pthread_mutex_unlock(pmutex_);
    }

private:
    pthread_mutex_t *pmutex_;
};

#endif