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

class XMNMemory : public NonCopyable
{
public:
    ~XMNMemory();

private:
    XMNMemory();

public:
    /**
     * @function    单例的生成器。
     * @paras       none 。
     * @return      单例的指针。
     * @author      xuchanglong
     * @time        2019-08-31      
     */
    static XMNMemory *GetInstance()
    {
        if (pinstance_ == nullptr)
        {
            if (pinstance_ == nullptr)
            {
                try
                {
                    pinstance_ = new XMNMemory();
                }
                catch (const std::exception &e)
                {
                    return nullptr;
                }
            }
        }
        return pinstance_;
    }

private:
    /**
     * @function    单例的销毁类。
     * @author      xuchanglong
     * @time        2019-08-01        
     */
    class DeleteXMNMemory
    {
    public:
        ~DeleteXMNMemory()
        {
            if (XMNMemory::pinstance_)
            {
                delete XMNMemory::pinstance_;
                XMNMemory::pinstance_ = nullptr;
            }
        }
    };

public:
    static XMNMemory *pinstance_;
    static DeleteXMNMemory deletememory;

public:
    /**
     * @function    申请内存。
     * @paras   bytecount   申请的内存的字节数。
     *          ismemset    是否对申请的内存进行 memset 。
     * @return  非 nullptr  申请成功。
     *          nullptr     申请失败。
     * @author  xuchanglong
     * @time    2019-08-31
    */
    void *AllocMemory(const size_t &bytecount, const bool &ismemset);

    /**
     * @function    释放内存。
     * @paras   pmemory   待释放的内存的首地址。
     * @return  none 。
     * @author  xuchanglong
     * @time    2019-08-31
    */
    void FreeMemory(void *pmemory);
};

#endif
