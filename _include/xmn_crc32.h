#ifndef XMOON__INCLUDE_XMN_CRC32_H_
#define XMOON__INCLUDE_XMN_CRC32_H_

#include "base/noncopyable.h"

#include <new>

class XMNCRC32 : public NonCopyable
{
private:
    XMNCRC32();
    ~XMNCRC32();

public:
    static XMNCRC32 *GetInstance()
    {
        if (pinstance_ == nullptr)
        {
            if (pinstance_ == nullptr)
            {
                try
                {
                    pinstance_ = new XMNCRC32();
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
    class DeleteXMNCRC32
    {
    public:
        ~DeleteXMNCRC32()
        {
            if (XMNCRC32::pinstance_)
            {
                delete XMNCRC32::pinstance_;
                XMNCRC32::pinstance_ = nullptr;
            }
        }
    };

private:
    static XMNCRC32 *pinstance_;
    static DeleteXMNCRC32 delXMNCRC32;

public:
    void InitCRC32Table();
    unsigned int Reflect(unsigned int ref, char ch);

    int GetCRC(unsigned char *buffer, unsigned int dwSize);

public:
    unsigned int crc32_table[256];
};

#endif
