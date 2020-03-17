/*****************************************************************************************
 * @function    含有嵌入式指针的内存池类。
 * @time    2019-10-31
 *****************************************************************************************/

#ifndef XMOON__INCLUDE_XMNMEMPOOL_H_
#define XMOON__INCLUDE_XMNMEMPOOL_H_

#include "base/noncopyable.h"

#include <unistd.h>
#include <cstdlib>

template <typename T>
class XMNMemPool : public NonCopyable
{
public:
    XMNMemPool() : kCount_(10), kMemBlockSize_(sizeof(T))
    {
        pfreehead_ = nullptr;
    }

    ~XMNMemPool(){};

public:
    /**
     * @function    从内存池中获取一个内存块，如果没有则再申请 kCount_ 个内存块。。
     * @paras   none 。
     * @ret 该连续内存的首地址。
    */
    void *Allocate()
    {
        AddressObj *pobj = nullptr;
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
        pobjtmp->next = pfreehead_;
        pfreehead_ = pobjtmp;
        return;
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
};

#endif