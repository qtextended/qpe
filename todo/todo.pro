TEMPLATE	= app
CONFIG		= qt warn_on release
DESTDIR		= ../bin
HEADERS	= mainwindow.h \
		  todotable.h \
		  todoentryimpl.h
SOURCES	= main.cpp \
		  mainwindow.cpp \
		  todotable.cpp \
		  todoentryimpl.cpp \

INTERFACES	= todoentry.ui

TARGET		= todo
INCLUDEPATH	+= ../library
DEPENDPATH	+= ../library
LIBS            += -lqpe
