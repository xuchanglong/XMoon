TARGET := spinlock.o
CFLAGS := -g -Wall -pthread

all:$(TARGET)

$(TARGET):spinlock.cc
	g++ -o $@ $^ -I $(CFLAGS)

.PHONY:clean
clean:
	rm -rf $(TARGET) 