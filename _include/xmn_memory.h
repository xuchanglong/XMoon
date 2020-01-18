/*****************************************************************************************
 * @function    内存分配类，仅仅是对 new 和 free 的简单的封装。
 * @notice      采用单例模式。
 * @author      xuchanglong
 * @time        2019-08-31   
 *****************************************************************************************/

#ifndef XMOON__INCLUDE_XMNMEMORY_H_
#define XMOON__INCLUDE_XMNMEMORY_H_

#include <vector>
#include <string.h>
#include <new>

#include "base/noncopyable.h"
#include "base/singletonbase.h"

class XMNMemory : public NonCopyable
{
    friend class SingletonBase<XMNMemory>;

private:
    ~XMNMemory();
    XMNMemory();

public:
    /**
     * @function    申请内存。
     * @paras   bytecount   申请的内存的字节数。
     *          ismemset    是否对申请的内存进行 memset 。
     * @ret  非 nullptr  申请成功。
     *          nullptr     申请失败。
     * @author  xuchanglong
     * @time    2019-08-31
    */
    void *AllocMemory(const size_t &bytecount, const bool &ismemset);

    /**
     * @function    释放内存。
     * @paras   pmemory   待释放的内存的首地址。
     * @ret  none 。
     * @author  xuchanglong
     * @time    2019-08-31
    */
    void FreeMemory(void *pmemory);
};

#endif
