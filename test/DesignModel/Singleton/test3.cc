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
    static T &GetInstance()
    {
        static T kT;
        return kT;
    }
};

int main()
{
    int &i1 = Singleton<int>::GetInstance();
    int &i2 = Singleton<int>::GetInstance();

    std::cout << "i1 = " << i1 << std::endl;
    std::cout << "i2 = " << i2 << std::endl;
    i1 = 20;
    std::cout << "i2 = " << i2 << std::endl;
    return 0;
}