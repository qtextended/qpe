TEMPLATE	= app
CONFIG		+= qt warn_on release
DESTDIR		= ../bin

HEADERS	= launcherview.h launcher.h categorylist.h
SOURCES	= main.cpp launcherview.cpp launcher.cpp categorylist.cpp

INCLUDEPATH	+= ../library
DEPENDPATH	+= ../library
LIBS            += -lqpe

TARGET		= launcher
