TEMPLATE	= app
CONFIG		= qt warn_on release
DESTDIR		= ../bin
HEADERS		= memory.h \
		  graph.h \
		  load.h \
		  storage.h \
		  version.h \
		  sysinfo.h
SOURCES		= main.cpp \
		  memory.cpp \
		  graph.cpp \
		  load.cpp \
		  storage.cpp \
		  version.cpp \
		  sysinfo.cpp
INTERFACES	= 

INCLUDEPATH	+= ../library
DEPENDPATH	+= ../library
LIBS            += -lqpe

TARGET		= sysinfo
