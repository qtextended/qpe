TEMPLATE	= app
CONFIG		= qt warn_on release
DESTDIR		= ../bin
HEADERS		= wordgame.h
SOURCES		= main.cpp \
		  wordgame.cpp
INTERFACES	= newgamebase.ui rulesbase.ui
TARGET		= wordgame
INCLUDEPATH	+= ../library
DEPENDPATH	+= ../library
LIBS            += -lqpe
