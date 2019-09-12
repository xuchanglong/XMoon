/*****************************************************************************************
 * 
 *  @function 用于自动释放互斥量，防止函数结束之后忘记释放。
 *  @author xuchanglong
 *  @time   2019-09-05
 * 
 *****************************************************************************************/

#ifndef XMOON__INClUDE_BASE_SINGLETONBASE_H_
#define XMOON__INClUDE_BASE_SINGLETONBASE_H_

#include <new>

template <typename T>
class SingleTonBase
{
public:
    static T *GetInstance()
    {
        if (pinstance_ == nullptr)
        {
            try
            {
                pinstance_ = new T();
            }
            catch (const std::exception &e)
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
    SingleTonBase() {}
    ~SingleTonBase() {}

private:
    static T *pinstance_;
    static DeleteT delT_;
};

template <typename T>
T *SingleTonBase<T>::pinstance_ = nullptr;

template <typename T>
typename SingleTonBase<T>::DeleteT SingleTonBase<T>::delT_;

#endif