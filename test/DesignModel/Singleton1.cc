#include <iostream>
#include <pthread.h>

template <typename T>
class Singleton
{
private:
    Singleton() {}
    ~Singleton() {}
    Singleton(const Singleton &) = delete;
    Singleton &operator=(const Singleton &) = delete;

public:
    static T *GetInstance()
    {
        if (pkInstance_ == nullptr)
        {
            pthread_mutex_lock(&kMtx_);
            try
            {
                pkInstance_ = new T();
            }
            catch (const std::exception &e)
            {
                pthread_mutex_unlock(&kMtx_);
                return nullptr;
            }
            pthread_mutex_unlock(&kMtx_);
        }
        return pkInstance_;
    }

private:
    class DeleteSingleton
    {
    public:
        ~DeleteSingleton()
        {
            if (Singleton::pkInstance_ != nullptr)
            {
                pthread_mutex_lock(&kMtx_);
                delete Singleton::pkInstance_;
                Singleton::pkInstance_ = nullptr;
                pthread_mutex_unlock(&kMtx_);
            }
        }
    };

private:
    static T *pkInstance_;
    static DeleteSingleton DeleteSingleton_;
    static pthread_mutex_t kMtx_;
};

template <typename T>
T *Singleton<T>::pkInstance_ = nullptr;
template <typename T>
typename Singleton<T>::DeleteSingleton Singleton<T>::DeleteSingleton_;
template <typename T>
pthread_mutex_t Singleton<T>::kMtx_ = PTHREAD_MUTEX_INITIALIZER;

int main()
{
    int *p1 = Singleton<int>::GetInstance();
    int *p2 = Singleton<int>::GetInstance();

    std::cout << "p1 = " << (long long)p1 << std::endl;
    std::cout << "p2 = " << (long long)p2 << std::endl;
    return 0;
}