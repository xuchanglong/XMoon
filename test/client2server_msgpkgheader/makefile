TARGET := testmsgpkgheader.o
INCLUDE := ../../_include
CFLAGS := -g -Wall

all:$(TARGET)

$(TARGET):msgpkgheader.cc ../../misc/xmn_crc32.cc
	g++ -o $@ $^  -I $(INCLUDE) $(CFLAGS)

.PHONY:clean
clean:
	rm -rf $(TARGET) 