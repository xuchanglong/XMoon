#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

struct PeopleInfo
{
    char name;
    size_t age;
};

const size_t kPeopleCount = 10;

int main()
{
    int fd = open("test.txt", O_CREAT | O_RDWR, 00777);
    PeopleInfo *pinfo = (PeopleInfo *)mmap(nullptr, sizeof(PeopleInfo) * kPeopleCount, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
    if (pinfo < 0)
    {
        std::cout << "Failed to create mmap." << std::endl;
        return -1;
    }
    close(fd);

    for (size_t i = 0; i < kPeopleCount; i++)
    {
        std::cout << "name = " << pinfo[i].name << "   "
                  << "age = " << pinfo[i].age << std::endl;
    }
    if (munmap((void *)pinfo, sizeof(PeopleInfo) * kPeopleCount) < 0)
    {
        std::cout << "Failed to delete mmap." << std::endl;
        return -2;
    }
    return 0;
}