#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <iostream>

struct sUserInfo
{
    int age;
    char name[20];
    char sex;
};

//这个进程读
int main(int argc, char *argv[]) 
{
    if (argc != 2)
    {
        std::cout << "Please specify the file." << std::endl;
        return -1;
    }
    int fd = open(argv[1], O_RDONLY, 0644);
    if (fd < 0)
    {
        std::cout << "open file failed." << std::endl;
        return -2;
    }
    struct sUserInfo student;
    struct sUserInfo *p = (struct sUserInfo *)mmap(NULL, sizeof(struct sUserInfo), PROT_READ, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED)
    {
        std::cout << "mmap failed." << std::endl;
        return -3;
    }
    close(fd);
    int i = 0;
    while (true)
    {
        printf("id = %d\tname = %s\t%c\n", p->age, p->name, p->sex);
        sleep(2);
    }
    int ret = munmap(p, sizeof(student));
    if (ret < 0)
    {
        std::cout << "munmap failed" << std::endl;
        return -4;
    }
    return 0;
}