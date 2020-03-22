/*****************************************************************************************
 * @function    含有嵌入式指针的内存池类。
 * @time    2019-10-31
 *****************************************************************************************/

#ifndef XMOON__INCLUDE_XMNMEMPOOL_H_
#define XMOON__INCLUDE_XMNMEMPOOL_H_

#include "base/noncopyable.h"
#include "base/singletonbase.h"

#include <unistd.h>
#include <cstdlib>
#include <atomic>
#include <pthread.h>

template <typename T>
class XMNMemPool : public NonCopyable
{
    friend class SingletonBase<XMNMemPool<T>>;

public:
    XMNMemPool() : kCount_(10), kMemBlockSize_(sizeof(T))
    {
        pfreehead_ = nullptr;
        memblockcount_ = 0;
        mtx_ = PTHREAD_MUTEX_INITIALIZER;
    }

    ~XMNMemPool()
    {
        AddressObj *pobj = nullptr;
        while (pfreehead_)
        {
            pobj = pfreehead_;
            pfreehead_ = pobj->next;
            free((void *)pobj);
        }
        // 静态初始化的锁无需销毁。
        //pthread_mutex_destroy(&mtx_);
    };

public:
    /**
     * @function    从内存池中获取一个内存块，如果没有则再申请 kCount_ 个内存块。。
     * @paras   none 。
     * @ret 该连续内存的首地址。
    */
    void *Allocate()
    {
        AddressObj *pobj = nullptr;

        pthread_mutex_lock(&mtx_);

        if (pfreehead_ == nullptr)
        {
            pfreehead_ = (AddressObj *)malloc(kMemBlockSize_ * kCount_);
            pobj = pfreehead_;
            for (size_t i = 0; i < kCount_ - 1; i++)
            {
                pobj->next = (AddressObj *)((char *)pobj + kMemBlockSize_);
                pobj = pobj->next;
            }
            pobj->next = nullptr;
        }
        pobj = pfreehead_;
        pfreehead_ = pobj->next;
        memblockcount_++;
        usedmemblockcount_++;

        pthread_mutex_unlock(&mtx_);
        return pobj;
    }

    /**
     * @function    待释放的大小为 size 的类的内存块。
     * @paras   phead   内存块的首地址。
     * @ret none 。
    */
    void DeAllocate(void *pobj)
    {
        if (pobj == nullptr)
        {
            return;
        }
        AddressObj *pobjtmp = (AddressObj *)pobj;

        pthread_mutex_lock(&mtx_);

        pobjtmp->next = pfreehead_;
        pfreehead_ = pobjtmp;
        //memblockcount_--;
        usedmemblockcount_--;

        pthread_mutex_unlock(&mtx_);
        return;
    }

    size_t MemBlockCount()
    {
        return memblockcount_;
    }

    size_t UsedMemBlockCount()
    {
        return usedmemblockcount_;
    }

private:
    struct AddressObj
    {
        AddressObj *next;
    };

private:
    /**
     * 始终指向下一个要被分配的内存块。
    */
    AddressObj *pfreehead_;

    /**
     * 当可用内存块用完之后一次性再分配的内存块的数量。
    */
    const size_t kCount_;

    /**
     * 每个内存块大小。
    */
    const size_t kMemBlockSize_;

    /**
     * 已申请的内存块的数量。
    */
    std::atomic<size_t> memblockcount_;

    /**
     * 已使用的内存块的数量。
    */
    std::atomic<size_t> usedmemblockcount_;

    /**
     * 内存池锁。
    */
    pthread_mutex_t mtx_;
};

#endif