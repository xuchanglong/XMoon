ROOTDIR = ../..
_INCLUDEDIR = $(ROOTDIR)/_include
APPDIR = $(ROOTDIR)/app
OBJDIR = ./obj

TARGET = main.o

$(TARGET) : main.cc $(OBJDIR)/xmoon_config.o
	g++ -I $(_INCLUDEDIR)  main.cc   $(OBJDIR)/xmoon_config.o  -o $@

$(OBJDIR)/xmoon_config.o: $(APPDIR)/xmoon_config.cc  $(_INCLUDEDIR)/xmoon_config.h  $(_INCLUDEDIR)/xmoon_global.h
	g++ -I $(_INCLUDEDIR) -c $(APPDIR)/xmoon_config.cc -o $@


