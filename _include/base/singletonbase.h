/*****************************************************************************************
 * 
 *  @function 单例模板类。
 *  @author xuchanglong
 *  @time   2019-09-12
 * 
 *****************************************************************************************/

#ifndef XMOON__INClUDE_BASE_SINGLETONBASE_H_
#define XMOON__INClUDE_BASE_SINGLETONBASE_H_

#include <new>

template <typename T>
class SingletonBase
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
    SingletonBase() {}
    ~SingletonBase() {}

private:
    static T *pinstance_;
    static DeleteT delT_;
};

template <typename T>
T *SingletonBase<T>::pinstance_ = nullptr;

template <typename T>
typename SingletonBase<T>::DeleteT SingletonBase<T>::delT_;

#endif