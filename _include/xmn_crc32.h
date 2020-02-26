/*****************************************************************************************
 * 
 *  @function CRC32 单字节校验算法。
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
     * @function 用 crc32_table 查找表来产生数据的CRC值
     * @paras buffer 待转换的数据的首地址。
     *        kSize 待转换的数据的字节数量。
     * @ret 最终的经过 crc32 转换之后的数据。
     * @time 2019-09-16
    */
    int GetCRC32(unsigned char *buffer, const size_t &kSize);

private:
    /**
     * @function 创建单字节的查表法所用到的表。
     * @paras none。
     * @ret none。
     * @time 2019-09-16
    */
    void InitCRC32Table();

    /**
     * @function 将 ref 各个位首尾交换。
     * @paras ref 待处理的数据。
     *        ch ref 数据需要交换的位数。
     * @time 2019-09-13
    */
    unsigned int Reflect(unsigned int ref, char ch);

private:
    unsigned int crc32_table[0xff];
};

#endif
