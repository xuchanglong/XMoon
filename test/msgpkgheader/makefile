TARGET := testmsgpkgheader.o
INCLUDE := ../../_include

all:$(TARGET)

$(TARGET):msgpkgheader.cc 
	g++ -o $@ $< -I $(INCLUDE)

.PHONY:clean
clean:
	rm -rf $(TARGET) 