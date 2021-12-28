TEMPLATE	= app
CONFIG		= qt warn_on release
DESTDIR		= ../bin
HEADERS		= helpbrowser.h
SOURCES		= helpbrowser.cpp \
		  main.cpp
INCLUDEPATH	+= ../library
DEPENDPATH	+= ../library
LIBS            += -lqpe
INTERFACES	= 
