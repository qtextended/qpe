TEMPLATE        = app

CONFIG          += qt warn_on release
DESTDIR         = ../bin

HEADERS		= showimg.h

SOURCES		= main.cpp \
		  showimg.cpp

TARGET          = showimg

INCLUDEPATH     += ../library
DEPENDPATH      += ../library
LIBS            += -lqpe

REQUIRES        = showimg

