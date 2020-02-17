#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

typedef struct
{
    char name;
    size_t age;
} PeopleInfo;

//map a normal file as shared mem:　
int main(int argc, char **argv)
{
    int fd;
    PeopleInfo *pmap;

    fd = open(argv[1], O_CREAT | O_RDWR, 00777);
    pmap = (PeopleInfo *)mmap(NULL, sizeof(PeopleInfo) * 10, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (pmap < 0)
    {
        std::cout << "Failed to create to mmap ." << std::endl;
        return -1;
    }
    for (size_t i = 0; i < 10; i++)
    {
        std::cout << "name = " << pmap[i].name << "  "
                  << "age = " << pmap[i].age << std::endl;
    }
    if (munmap(pmap, sizeof(PeopleInfo) * 10) < 0)
    {
        std::cout << "Failed to delete to mmap ." << std::endl;
        return -2;
    }
    return 0;
}