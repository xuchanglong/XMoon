TARGET := main.o
INCLUDEDIR := ../../_include
MISCDIR := ../../misc

all:$(TARGET)

$(TARGET):main.cc mempool.o
	g++ -o $@ $^ -I $(INCLUDEDIR)

mempool.o:$(MISCDIR)/xmn_mempool.cc $(INCLUDEDIR)/xmn_mempool.h
	g++ -c $< -o $@ -I $(INCLUDEDIR)

.PHONY:clean
clean:
	rm -rf $(TARGET)  