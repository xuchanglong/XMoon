TARGET_S := server.o
TARGET_C := client.o
TARGET := $(TARGET_S) $(TARGET_C)
CFLAGS := -g -Wall

all:$(TARGET)

$(TARGET_S):server.cc
	g++ -o $@ $< $(CFLAGS)
$(TARGET_C): client.cc
	g++ -o $@ $< $(CFLAGS)

.PHONY:clean
clean:
	rm -rf $(TARGET_S) $(TARGET_C)