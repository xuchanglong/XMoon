#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <iostream>

struct sUserInfo
{
    int age;
    char name[20];
    char sex;
};

//这个进程用于创建映射区进行写。
int main(int argc, char *argv[]) 
{
    if (argc != 2)
    {
        std::cout << "Please specify the file." << std::endl;
        return -1;
    }
    struct sUserInfo student = {10, "xiaoming", 'm'};
    int fd = open(argv[1], O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
    {
        std::cout << "open file failed." << std::endl;
        return -2;
    }
    ftruncate(fd, sizeof(struct sUserInfo)); //文件拓展大小。
    //创建一个结构体大小的共享映射区。共享映射区我们可以当做数组区看待。
    struct sUserInfo *p = (struct sUserInfo *)mmap(NULL, sizeof(struct sUserInfo), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); 
    if (p == MAP_FAILED)
    {
        std::cout << "mmap failed." << std::endl;
        return -3;
    }
    close(fd); //关闭不用的文件描述符。
    while (true)
    {
        memcpy(p, &student, sizeof(student));
        student.age++;
        sleep(1);
    }
    int ret = munmap(p, sizeof(student));
    if (ret < 0)
    {
        std::cout << "munmap failed" << std::endl;
        return -4;
    }
    return 0;
}