TARGET_S := server.o
TARGET_C := client.o
TARGET := $(TARGET_S) $(TARGET_C)

all:$(TARGET)

$(TARGET_S):server.cc
	g++ -o $@ $<
$(TARGET_C): client.cc
	g++ -o $@ $<

.PHONY:clean
clean:
	rm -rf $(TARGET_S) $(TARGET_C)