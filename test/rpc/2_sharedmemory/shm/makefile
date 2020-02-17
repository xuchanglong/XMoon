TARGET := ./obj/server ./obj/client

all:$(TARGET)

$(TARGET):server.cc client.cc
	g++ server.cc -o ./obj/server -pthread -g
	g++ client.cc -o ./obj/client -pthread -g

.PHONY:clean
clean:
	rm -rf $(TARGET)