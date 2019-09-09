TARGET := shock_epoll.o 

all:$(TARGET)

$(TARGET):shock_epoll.cc
	g++ -o $@ $<

.PHONY:clean
clean:
	rm -rf $(TARGET) 