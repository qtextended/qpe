TEMPLATE	= app
CONFIG		+= qt warn_on release
DESTDIR		= ../bin

HEADERS	= dicttool.h
SOURCES	= dicttool.cpp main.cpp

INTERFACES	= dicttoolbase.ui

INCLUDEPATH	+= ../library
DEPENDPATH	+= ../library
LIBS            += -lqpe

TARGET		= dictionary
