TEMPLATE	= lib
CONFIG		+= qt warn_on release
HEADERS	=   volume.h volumeappletimpl.h
SOURCES	=   volume.cpp volumeappletimpl.cpp
TARGET		= volumeapplet
DESTDIR		= ../../plugins/applets
INCLUDEPATH     += ../../library ..
DEPENDPATH      += ../../library ..
LIBS            += -lqpe
VERSION		= 1.0.0