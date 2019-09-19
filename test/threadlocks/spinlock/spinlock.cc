#include <unistd.h>
#include <pthread.h>
#include <iostream>

static pthread_spinlock_t spinlock;
static int value = 0;

void *FirstThreadFunc(void *arg)
{
    std::cout << "first thread begin run." << std::endl;
    while (true)
    {
        pthread_spin_lock(&spinlock);
        std::cout << "first thread get spinlock." << std::endl;
        value++;
        std::cout << "value is " << value << std::endl;
        sleep(6);
        pthread_spin_unlock(&spinlock);
        std::cout << std::endl;
    }
    return nullptr;
}

void *SecondThreadFunc(void *arg)
{
    std::cout << "second thread begin run." << std::endl;
    while (true)
    {
        pthread_spin_lock(&spinlock);
        std::cout << "second thread get spinlock." << std::endl;
        value--;
        std::cout << "value is " << value << std::endl;
        sleep(3);
        pthread_spin_unlock(&spinlock);
        std::cout << std::endl;
    }
    return nullptr;
}

int main(int argC, char *arg[])
{

    int err;
    pthread_t tid1, tid2;

    pthread_spin_init(&spinlock, 0);

    err = pthread_create(&tid1, nullptr, FirstThreadFunc, nullptr);
    if (err != 0)
    {
        std::cout << "thread creation failed." << std::endl;
        return 1;
    }

    err = pthread_create(&tid2, nullptr, SecondThreadFunc, nullptr);
    if (err != 0)
    {
        std::cout << "thread creation failed." << std::endl;
        return 2;
    }

    pthread_join(tid1, nullptr);
    pthread_join(tid2, nullptr);

    return 0;
}