#include "xmn_mempool.h"

XMNMemPool::XMNMemPool(size_t trunkcount)
{
    trunkcount_ = trunkcount;
    pfreehead_ = nullptr;
}

XMNMemPool::~XMNMemPool()
{
    ;
}

void *XMNMemPool::Allocate(size_t size)
{
    AddressObj *pobj = nullptr;
    if (pfreehead_ == nullptr)
    {
        pfreehead_ = (AddressObj *)malloc(size * trunkcount_);
        pobj = pfreehead_;
        for (size_t i = 0; i < trunkcount_ - 1; i++)
        {
            pobj->next = (AddressObj *)((char *)pobj + size);
            pobj = pobj->next;
        }
        pobj->next = nullptr;
    }
    pobj = pfreehead_;
    pfreehead_ = pobj->next;
    return pobj;
}

void XMNMemPool::DeAllocate(void *phead)
{
    AddressObj *pobj = (AddressObj *)phead;
    pobj->next = pfreehead_;
    pfreehead_ = pobj;
}