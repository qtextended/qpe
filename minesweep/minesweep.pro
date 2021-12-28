TEMPLATE	= app
CONFIG		= qt warn_on release
DESTDIR		= ../bin
HEADERS		= minefield.h \
		  minesweep.h
SOURCES		= main.cpp \
		  minefield.cpp \
		  minesweep.cpp
INCLUDEPATH	+= ../library
DEPENDPATH	+= ../library
LIBS            += -lqpe
INTERFACES	= 
