#include <pthread.h>
#include <iostream>

void *thread_func_read_one(void *arg);
void *thread_func_read_two(void *arg);
void *thread_func_write_one(void *arg);
void *thread_func_write_two(void *arg);

using threadfunc = void *(*)(void *arg);

static const threadfunc threadfuncsum[] =
    {
        &thread_func_read_one,
        &thread_func_read_two,
        &thread_func_write_one,
        &thread_func_write_two};

int main()
{
    return 0;
}