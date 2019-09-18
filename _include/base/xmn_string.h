#ifndef XMOON__INCLUDE_BASE_XMN_STRING_H_
#define XMOON__INCLUDE_BASE_XMN_STRING_H_
//===========================  内存比较函数  ===========================//
int memcmp(const void *cs, const void *ct, size_t count)
{
    const unsigned char *su1, *su2;
    int res = 0;

    for (su1 = (const unsigned char *)cs, su2 = (const unsigned char *)ct; 0 < count; ++su1, ++su2, count--)
        if ((res = *su1 - *su2) != 0)
            break;
    return res;
}

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

//===========================  内存复制函数  ===========================//
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

char *strcpy(char *dest, const char *src)
{
    char *tmp = dest;

    while ((*dest++ = *src++) != '\0')
        /* nothing */;
    return tmp;
}

/**
 * @function：从src拷贝count个字符到dest中。若src的buffer长度小于count，则dest后续字符用'\0'补充。
 * @paras: dest 被赋值的内存。
 *         src  待拷贝的内存。
 *         count 赋值的字节数量。
 * @return: 目标内存赋值完数据之后下一个位置。
 * @notice:if ((*tmp = *src) != 0) 这里比较用的是“0”，因为和字符比较时，是双方的ASCII码值比较，故写 0 亦可。
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
void *memset(void *s, int c, size_t count)
{
    char *xs = (char *)s;

    while (count--)
    {
        *xs++ = c;
    }
    return s;
}

size_t strlen(const char *s)
{
    const char *sc;

    for (sc = s; *sc != '\0'; ++sc)
        /* nothing */;
    return sc - s;
}

/**
 * @function 翻转字符串。
 * @paras str 待翻转的字符串。
 * @raturn 翻转完之后的字符串。
 * @author xuchanglong
 * @time 2019-09-19
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