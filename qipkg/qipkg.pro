DESTDIR		= ../bin
TEMPLATE	= app
CONFIG		= qt warn_on release
HEADERS		= packagemanager.h
SOURCES		= packagemanager.cpp \
		  main.cpp
INCLUDEPATH	+= ../library
DEPENDPATH	+= ../library
LIBS            += -lqpe
INTERFACES	= packagemanagerbase.ui pkdesc.ui pkfind.ui pksettings.ui
TARGET		= qipkg
