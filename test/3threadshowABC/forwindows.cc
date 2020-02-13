#include <stdio.h>
#include <process.h>
#include <windows.h>

// 线程个数
const int THREAD_NUM = 3;

// 循环次数
const int LOOP = 10;

// 子线程同步事件
HANDLE g_hThreadEvent[THREAD_NUM];
// 主线程与子线程同步
HANDLE g_Semaphore;
int g_Count = 0;

unsigned int __stdcall ThreadFunction(void *pPM)
{
    int num = *(int *)pPM;
    // 信号量++
    ReleaseSemaphore(g_Semaphore, 1, NULL);

    for (int i = 0; i < LOOP; i++)
    {
        // 等待该事件有效。
        WaitForSingleObject(g_hThreadEvent[num], INFINITE);
        g_Count++;

        printf("第%d次 线程ID:%3d,线程打印:%c\n ", g_Count, GetCurrentThreadId(), num + 'A');
        // 置位下一个事件有效。
        SetEvent(g_hThreadEvent[(num + 1) % THREAD_NUM]);
    }

    return 0;
}

int main(void)
{
    int i = 0;
    HANDLE hThreadHandle[THREAD_NUM];
    // 形参1：安全控制，一般为NULL
    // 形参2：资源的初始值。
    // 形参3：最大的资源数量。
    // 形参4：该信号量的名称。
    g_Semaphore = CreateSemaphore(NULL, 0, 1, NULL); // 当前0个资源，最大允许1个同时访问

    for (i = 0; i < THREAD_NUM; i++)
    {
        // 形参1：安全控制。
        // 形参2：该事件是手动复原（TRUE）还是自动复原（FALSE）。
        // 形参3：指定事件的初始状态。若为TRUE，则为有信号状态，若为FALSE，则为无信号状态。
        // 形参4：指定该事件的名称。
        g_hThreadEvent[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
    }

    for (i = 0; i < THREAD_NUM; i++)
    {
        // 创建线程
        hThreadHandle[i] = (HANDLE)_beginthreadex(nullptr, 0, ThreadFunction, &i, 0, nullptr);
        // 每次创建完线程，该信号量都会等待，直至线程执行到释放信号量的代码为止。
        WaitForSingleObject(g_Semaphore, INFINITE);
    }
    // 代码执行到这里，说明各个线程都已经执行到WaitForSingleObject，在等待确认信号。
    // 置位线程1的事件。
    SetEvent(g_hThreadEvent[0]);
    // 等待所有线程返回。
    WaitForMultipleObjects(THREAD_NUM, hThreadHandle, true, INFINITE);

    for (i = 0; i < THREAD_NUM; i++)
    {
        CloseHandle(hThreadHandle[i]);
        CloseHandle(g_hThreadEvent[i]);
    }

    CloseHandle(g_Semaphore);

    system("pause");
    return 0;
}