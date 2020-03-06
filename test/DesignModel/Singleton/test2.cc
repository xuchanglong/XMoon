#include <iostream>
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
                delete Singleton::pkInstance_;
                Singleton::pkInstance_ = nullptr;
            }
        }
    };

private:
    static DeleteSingleton DeleteSingleton_;
    static T *pkInstance_;
};

template <typename T>
T *Singleton<T>::pkInstance_ = new T();
template <typename T>
typename Singleton<T>::DeleteSingleton Singleton<T>::DeleteSingleton_;

int main()
{
    int *p1 = Singleton<int>::GetInstance();
    int *p2 = Singleton<int>::GetInstance();

    std::cout << "p1 = " << (long long)p1 << std::endl;
    std::cout << "p2 = " << (long long)p2 << std::endl;
    return 0;
}