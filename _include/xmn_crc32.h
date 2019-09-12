#ifndef XMOON__INCLUDE_XMN_CRC32_H_
#define XMOON__INCLUDE_XMN_CRC32_H_

#include "base/noncopyable.h"
#include "base/singletonbase.h"

#include <new>

class XMNCRC32 : public NonCopyable
{
    friend class SingletonBase<XMNCRC32>;

private:
    XMNCRC32();
    ~XMNCRC32();

public:
    void InitCRC32Table();
    unsigned int Reflect(unsigned int ref, char ch);

    int GetCRC(unsigned char *buffer, unsigned int dwSize);

public:
    unsigned int crc32_table[0xff];
};

#endif
