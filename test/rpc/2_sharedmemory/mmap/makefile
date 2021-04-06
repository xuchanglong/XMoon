TARGET := ./obj/test1.o ./obj/test2.o

all:$(TARGET)

$(TARGET):test1.cc test2.cc
	g++ test1.cc -o ./obj/test1.o -g
	g++ test2.cc -o ./obj/test2.o -g

.PHONY:clean
clean:
	rm -rf $(TARGET)