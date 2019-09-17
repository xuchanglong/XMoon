TARGET := readwritelock.o
CFLAGS := -g -Wall -pthread

all:$(TARGET)

$(TARGET):readwritelock.cc
	g++ -o $@ $^  -I  $(CFLAGS)

.PHONY:clean
clean:
	rm -rf $(TARGET) 