#ifndef XMOON__INClUDE_BASE_SINGLETONBASE_H_
#define XMOON__INClUDE_BASE_SINGLETONBASE_H_

#include <new>

template <typename T>
class SingleTonBase
{
public:
    static T *GetInstance()
    {
        if (pinstance_==nullptr)
        {
            try
            {
                pinstance_ = new T();
            }
            catch(const std::exception& e)
            {
                return nullptr;
            }
        }
        return pinstance_;
    }

private:
    class DeleteT
    {
    public:
        ~DeleteT()
        {
            if (pinstance_)
            {
                delete pinstance_;
                pinstance_ = nullptr;
            }
        }
    };

private:
    SingleTonBase(){}
    ~SingleTonBase(){}

private:
    static T *pinstance_;
    static DeleteT delT_;
};

template <typename T>
T *SingleTonBase<T>::pinstance_ = nullptr;

template <typename T>
SingleTonBase<T>::DeleteT SingleTonBase<T>::delT_;

#endif