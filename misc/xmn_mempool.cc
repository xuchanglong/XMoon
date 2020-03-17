#include "xmn_mempool.h"

XMNMemPool::XMNMemPool(const size_t kCount) : kCount_(kCount), kMemBlockSize_(sizeof(T))
{
    pfreehead_ = nullptr;
}

XMNMemPool::~XMNMemPool()
{
    ;
}

void *XMNMemPool::Allocate()
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

void XMNMemPool::DeAllocate(void *pobj)
{
    AddressObj *pobjtmp = (AddressObj *)pobj;
    pobjtmp->next = pfreehead_;
    pfreehead_ = pobjtmp;
}