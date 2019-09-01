#include "xmn_memory.h"

XMNMemory *XMNMemory::pinstance_ = nullptr;
XMNMemory::DeleteXMNMemory XMNMemory::deletememory;

XMNMemory::XMNMemory()
{
    ;
}

XMNMemory::~XMNMemory()
{
    ;
}

void *XMNMemory::AllocMemory(const size_t &bytecount, const bool &ismemset)
{
    char *presult = nullptr;
    /**
     * TODO：需要确定标准C++的异常处理是否是这样写。
    */
    try
    {
        presult = new char[bytecount];
    }
    catch (const std::exception &e)
    {
        return nullptr;
    }

    if (ismemset)
    {
        memset(presult, 0, sizeof(char) * bytecount);
    }
    return (void *)presult;
}

void XMNMemory::FreeMemory(void *pmemory)
{
    if (pmemory == nullptr)
    {
        return ;
    }
    /**
     * TODO：需要确定标准C++的异常处理是否是这样写。
    */
    try
    {
        delete pmemory;
        pmemory = nullptr;
    }
    catch(const std::exception& e)
    {
        return ;
    }
    return ;
}
