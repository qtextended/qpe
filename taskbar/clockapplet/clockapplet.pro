TEMPLATE	= lib
CONFIG		+= qt warn_on release
HEADERS	=   clock.h clockappletimpl.h
SOURCES	=   clock.cpp clockappletimpl.cpp
TARGET		= clockapplet
DESTDIR		= ../../plugins/applets
INCLUDEPATH     += ../../library ..
DEPENDPATH      += ../../library ..
LIBS            += -lqpe
VERSION		= 1.0.0