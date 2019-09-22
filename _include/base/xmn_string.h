#ifndef XMOON__INCLUDE_BASE_XMN_STRING_H_
#define XMOON__INCLUDE_BASE_XMN_STRING_H_

using size_t = unsigned long; 

//===========================  内存比较函数  ===========================//
/**
 * @function    内存比较函数。
 * @paras   cs、ct 待比较的内存。
 *          count   待比较的字节数量。
 * @return  0   二者指定字节数的内存相同。
 *          非0 二者指定字节数的内存不相同。s
 * @author  xuchanglong
 * @time    2019-09-19
*/
int memcmp(const void *cs, const void *ct, size_t count)
{
    const unsigned char *su1, *su2;
    int res = 0;

    for (su1 = (const unsigned char *)cs, su2 = (const unsigned char *)ct; 0 < count; ++su1, ++su2, count--)
        if ((res = *su1 - *su2) != 0)
            break;
    return res;
}

/**
 * @function    字符串比较函数。
 * @paras   cs、ct 待比较的字符串。
 * @return  0   两个字符串相同。
 *          -1  对于第1个不同的字符,cs < ct 。
 *          1   对于第1个不同的字符,cs > ct 。
 * @author  xuchanglong
 * @time    2019-09-19
*/
int strcmp(const char *cs, const char *ct)
{
    unsigned char c1, c2;

    while (true)
    {
        c1 = *cs++;
        c2 = *ct++;
        if (c1 != c2)
        {
            return c1 < c2 ? -1 : 1;
        }
        if (!c1)
        {
            break;
        }
    }
    return 0;
}

//===========================  内存拷贝函数  ===========================//
/**
 * @function    内存拷贝函数。
 * @paras   dest 目的内存。
 *          src 待拷贝的内存。
 * @return  目标内存赋值完数据之后下一个位置。
 * @author  xuchanglong
 * @time    2019-09-19
*/
void *memcpy(void *dest, const void *src, size_t count)
{
    char *tmp = (char *)dest;
    const char *s = (const char *)src;

    while (count--)
    {
        *tmp++ = *s++;
    }
    return dest;
}

/**
 * @function    字符串拷贝函数。
 * @paras   dest 目的内存。
 *          src 待拷贝的字符串。
 * @return  dest 的开始地址。
 * @author  xuchanglong
 * @time    2019-09-19
*/
char *strcpy(char *dest, const char *src)
{
    char *tmp = dest;

    while ((*dest++ = *src++) != '\0')
        /* nothing */;
    return tmp;
}

/**
 * @function    从src拷贝count个字符到dest中。若src的buffer长度小于count，则dest后续字符用'\0'补充。
 * @paras   dest 被赋值的内存。
 *          src  待拷贝的内存。
 *          count 赋值的字节数量。
 * @return  目标内存赋值完数据之后下一个位置。
 * @author  xuchanglong
 * @time    2019-09-19
 * @notice  if ((*tmp = *src) != 0) 这里比较用的是“0”，因为和字符比较时，是双方的ASCII码值比较，故写 0 亦可。
*/
char *strncpy(char *dest, const char *src, size_t count)
{
    char *tmp = dest;

    while (count)
    {
        if ((*tmp = *src) != 0)
            src++;
        tmp++;
        count--;
    }
    return dest;
}

//===========================  内存赋值、计算长度、翻转函数  ===========================//
/**
 * @function    内存批量赋值函数。
 * @paras   s 待赋值的内存。
 *          c 值。
 *          count 待赋值的字节数。
 * @return  待赋值的内存的首地址。
 * @author  xuchanglong
 * @time    2019-09-19
*/
void *memset(void *s, int c, size_t count)
{
    char *xs = (char *)s;

    while (count--)
    {
        *xs++ = c;
    }
    return s;
}

/**
 * @function    字符串计数函数
 * @paras   s 待计数的内存
 * @return  字符串的长度。
 * @author  xuchanglong
 * @time    2019-09-19
*/
size_t strlen(const char *s)
{
    const char *sc;

    for (sc = s; *sc != '\0'; ++sc)
        /* nothing */;
    return sc - s;
}

/**
 * @function    翻转字符串。
 * @paras   str 待翻转的字符串。
 * @raturn  翻转完之后的字符串。
 * @author  xuchanglong
 * @time    2019-09-19
*/
char *strrev(char *str)
{
    char *p1, *p2;

    if (!str || !*str)
    {
        return str;
    }
    for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
    {
        *p1 ^= *p2;
        *p2 ^= *p1;
        *p1 ^= *p2;
    }
    return str;
}

#endif