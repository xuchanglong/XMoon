#ifndef XMOON__INClUDE_BASE_SINGLETONBASE_H_
#define XMOON__INClUDE_BASE_SINGLETONBASE_H_

template <typename T>
class SingletonBase
{
private:
    SingletonBase() {}
    ~SingletonBase() {}

public:
    static T &GetInstance()
    {
        static T kT;
        return kT;
    }
};

#endif