TEMPLATE	= lib
CONFIG		+= qt warn_on release
HEADERS	=   battery.h batterystatus.h batteryappletimpl.h
SOURCES	=   battery.h batterystatus.cpp batteryappletimpl.cpp
TARGET		= batteryapplet
DESTDIR		= ../../plugins/applets
INCLUDEPATH     += ../../library ..
DEPENDPATH      += ../../library ..
LIBS            += -lqpe
VERSION		= 1.0.0
