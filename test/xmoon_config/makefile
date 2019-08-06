ROOTDIR = ../..
_INCLUDEDIR = $(ROOTDIR)/_include
APPDIR = $(ROOTDIR)/app
OBJDIR = ./obj

TARGET = main.o
CFLAGS = -g -Wall

$(TARGET) : main.cc $(OBJDIR)/xmoon_config.o
	g++ -I $(_INCLUDEDIR)  main.cc   $(OBJDIR)/xmoon_config.o  -o $@ $(CFLAGS)

$(OBJDIR)/xmoon_config.o: $(APPDIR)/xmoon_config.cc  $(_INCLUDEDIR)/xmoon_config.h  $(_INCLUDEDIR)/xmoon_global.h
	g++ -I $(_INCLUDEDIR) -c $(APPDIR)/xmoon_config.cc -o $@ $(CFLAGS)

.PHONY:clean
clean:
	rm -rf $(OBJDIR)/*


