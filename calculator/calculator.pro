DESTDIR		= ../bin
TEMPLATE	= app
CONFIG		= qt warn_on release
HEADERS		= calculatorimpl.h
SOURCES		= calculatorimpl.cpp \
		  main.cpp
INCLUDEPATH	+= ../library
DEPENDPATH	+= ../library
LIBS            += -lqpe
INTERFACES	= calculator.ui
TARGET		= calculator
