TARGET := forlinux.o

all:$(TARGET)

$(TARGET):forlinux.cc
	g++ -o $@ $< -pthread

.PHONY:clean
clean:
	rm -rf $(TARGET)  