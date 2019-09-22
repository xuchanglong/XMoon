TARGET := main.o

all:$(TARGET)

$(TARGET):main.cc
	g++ -o $@ $<

.PHONY:clean
clean:
	rm -rf $(TARGET)  