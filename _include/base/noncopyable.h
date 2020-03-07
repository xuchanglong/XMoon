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
 * 这里使用 protected 是为了防止该类被单独的创建对象。
*/
protected:
    NonCopyable() = default;
    ~NonCopyable() = default;
};

#endif