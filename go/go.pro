DESTDIR		= ../bin
TEMPLATE	= app
CONFIG		= qt warn_on release
HEADERS		= amigo.h \
		  go.h \
		  goplayutils.h \
		  gowidget.h
SOURCES		= amigo.c \
		goplayer.c \
		goplayutils.c \
		killable.c \
		gowidget.cpp \
		main.cpp
INCLUDEPATH	+= ../library
DEPENDPATH	+= ../library
LIBS            += -lqpe
TARGET		= go
