/*****************************************************************************************
 * @function    含有嵌入式指针的内存池类。
 * @time    2019-10-31
 *****************************************************************************************/

#ifndef XMOON__INCLUDE_XMNMEMPOOL_H_
#define XMOON__INCLUDE_XMNMEMPOOL_H_

#include "base/noncopyable.h"

#include <unistd.h>
#include <cstdlib>

class XMNMemPool : public NonCopyable
{
public:
    XMNMemPool(size_t trunkcount = 5);
    ~XMNMemPool();

public:
    /**
     * @function    分配 trunkcount_ 个大小为 size 的类的连续内存。
     * @paras   size    类的大小。
     * @ret 该连续内存的首地址。
    */
    void *Allocate(size_t size);

    /**
     * @function    待释放的大小为 size 的类的内存块。
     * @paras   phead   内存块的首地址。
     * @ret none 。
    */
    void DeAllocate(void *phead);

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
    size_t trunkcount_;
};

#endif