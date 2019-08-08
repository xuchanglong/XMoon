ROOTDIR = ../..
_INCLUDEDIR = $(ROOTDIR)/_include
APPDIR = $(ROOTDIR)/app
OBJDIR = ./obj

TARGET = main.o
CFLAGS = -g -Wall

$(TARGET) : main.cc $(OBJDIR)/xmoon_setting.o
	g++ -I $(_INCLUDEDIR)  main.cc   $(OBJDIR)/xmoon_setting.o  -o $@ $(CFLAGS)

$(OBJDIR)/xmoon_setting.o: $(APPDIR)/xmoon_setting.cc  $(_INCLUDEDIR)/xmoon_setting.h  
	g++ -I $(_INCLUDEDIR) -c $(APPDIR)/xmoon_setting.cc -o $@ $(CFLAGS)

.PHONY:clean
clean:
	rm -rf $(OBJDIR)/*


