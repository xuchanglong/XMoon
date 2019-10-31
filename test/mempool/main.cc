#include "xmn_mempool.h"

#include <iostream>

class A
{
public:
    int i_ = 0;
    int t_ = 0;

public:
    static XMNMemPool mempool_;

public:
    static void *operator new(size_t size)
    {
        return mempool_.Allocate(size);
    }
    static void operator delete(void *phead)
    {
        return mempool_.DeAllocate(phead);
    }
};

XMNMemPool A::mempool_;

int main()
{
    A *asum[15];
    for (size_t i = 0; i < 15; i++)
    {
        asum[i] = new A();
        std::cout << asum[i] << std::endl;
    }

    return 0;
}