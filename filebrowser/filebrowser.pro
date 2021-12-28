TEMPLATE	= app
CONFIG		= qt warn_on release
DESTDIR		= ../bin
HEADERS		= inlineedit.h \
		  filebrowser.h
SOURCES		= filebrowser.cpp \
		  inlineedit.cpp \
		  main.cpp
INCLUDEPATH	+= ../library
DEPENDPATH	+= ../library
LIBS            += -lqpe
INTERFACES	= 
