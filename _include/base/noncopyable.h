#ifndef XMOON_MISC_NONCOPYABLE_
#define XMOON_MISC_NONCOPYABLE_

class NonCopyable
{
public:
    NonCopyable(const NonCopyable &kObj) = delete;
    NonCopyable &operator=(const NonCopyable &kObj) = delete;
protected:
    NonCopyable() = default;
    ~NonCopyable() = default;
};

#endif