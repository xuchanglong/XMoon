
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static pthread_spinlock_t spinlock;
static int test_value = 0;

int UDP_first_thread()
{
    int i, ret;

    printf("UDP_first_thread begin\n");
    for (i = 0; i < 10; i++)
    {
        pthread_spin_lock(&spinlock);
        test_value++;
        printf("test_value %d\n", test_value);
        sleep(5);
        pthread_spin_unlock(&spinlock);
    }
    printf("UDP_first_thread end\n");
    return ret;
}

int UDP_second_thread()
{
    int i, ret;

    printf("UDP_second_thread begin\n");
    for (i = 0; i < 10; i++)
    {
        pthread_spin_lock(&spinlock);
        test_value--;
        printf("test_value %d\n", test_value);
        sleep(3);
        pthread_spin_unlock(&spinlock);
    }
    printf("UDP_second_thread end\n");
    return ret;
}

int main(int argC, char *arg[])
{

    int err;
    pthread_t tid1, tid2;

    pthread_spin_init(&spinlock, 0);
    //----------------创建UDP服务端线程----------------
    err = pthread_create(&tid1, NULL, UDP_first_thread, NULL);
    if (err != 0)
    {
        perror(" fail to create thread ");
        return -1;
    }
    sleep(1);

    err = pthread_create(&tid1, NULL, UDP_second_thread, NULL);
    if (err != 0)
    {
        perror(" fail to create thread ");
        return -1;
    }

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    printf("main end\n");

    return 0;
}