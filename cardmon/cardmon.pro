TEMPLATE	= lib
CONFIG		+= qt warn_on release
HEADERS	=	cardmon.h cardmonimpl.h
SOURCES	=	cardmon.cpp cardmonimpl.cpp
TARGET		= cardmonapplet
DESTDIR		= ../plugins/applets
INCLUDEPATH     += ../library ../taskbar
DEPENDPATH      += ../library ../taskbar
LIBS            += -lqpe
VERSION		= 1.0.0
