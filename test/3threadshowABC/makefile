TARGET := forlinux.o

all:$(TARGET)

$(TARGET):forlinux.cc
	g++ -o ./obj/$@ $< -pthread -g

.PHONY:clean
clean:
	rm -rf ./obj/$(TARGET)  