/*****************************************************************************************
 * @function    内存池模板。
 * @time    2019-10-31
 *****************************************************************************************/

#ifndef XMOON__INCLUDE_XMNMEMPOOL_H_
#define XMOON__INCLUDE_XMNMEMPOOL_H_

#include "base/noncopyable.h"

template <typename T>
class XMNMemPool : public NonCopyable
{
public:
    XMNMemPool(size_t trunkcount = 5)
    {
        trunkcount_ = trunkcount;
        pfreehead_ = nullptr;
    }
    ~XMNMemPool();

public:
    void Allocate(size_t size);
    void DeAllocate(void *phead);

private:
    struct AddressObj
    {
        AddressObj *next;
    };

private:
    AddressObj *pfreehead_;
    size_t trunkcount_;
};

#endif