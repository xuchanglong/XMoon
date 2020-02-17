TARGET := ./obj/mmap_test1.o ./obj/mmap_test2.o

all:$(TARGET)

$(TARGET):mmap_test1.cc mmap_test2.cc
	g++ mmap_test1.cc -o ./obj/mmap_test1.o -g
	g++ mmap_test2.cc -o ./obj/mmap_test2.o -g

.PHONY:clean
clean:
	rm -rf $(TARGET)