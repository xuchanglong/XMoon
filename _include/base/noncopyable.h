#ifndef XMOON_MISC_NONCOPYABLE_H_
#define XMOON_MISC_NONCOPYABLE_H_

class NonCopyable
{
public:
    /**
     * 该类禁止生成默认的拷贝构造函数和赋值构造函数。
    */
    NonCopyable(const NonCopyable &kObj) = delete;
    NonCopyable &operator=(const NonCopyable &kObj) = delete;
/**
 * TODO：需要确定这里使用 protected 的原因。
*/
protected:
    NonCopyable() = default;
    ~NonCopyable() = default;
};

#endif