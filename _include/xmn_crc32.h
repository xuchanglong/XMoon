/*****************************************************************************************
 * 
 *  @function CRC32 校验算法。
 *  @author xuchanglong
 *  @time   2019-09-13
 *  @reference link：https://www.cnblogs.com/esestt/archive/2007/08/09/848856.html
 * 
 *****************************************************************************************/
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
    /**
     * @function 用crc32_table寻找表来产生数据的CRC值
     * TODO：后续补充。
    */
    int GetCRC(unsigned char *buffer, unsigned int dwSize);

private:
    /**
     * TODO：后续补充。
    */
    void InitCRC32Table();

    /**
     * @function 首尾位交换。
     * @paras ref 待处理的数据。
     *        ch TODO：后续补充。
     * @author xuchanglong
     * @time 2019-09-13
    */
    unsigned int Reflect(unsigned int ref, char ch);

private:
    unsigned int crc32_table[0xff];
};

#endif
