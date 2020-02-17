TARGET := ./obj/mmap_test1 ./obj/mmap_test2

all:$(TARGET)

$(TARGET):mmap_test1.cc mmap_test2.cc
	g++ mmap_test1.cc -o ./obj/mmap_test1 -g
	g++ mmap_test2.cc -o ./obj/mmap_test2 -g

.PHONY:clean
clean:
	rm -rf $(TARGET)