TEMPLATE	= lib
CONFIG		+= qt warn_on release
HEADERS	=   keyboard.h \
	    ../pickboard/pickboardcfg.h \
	    ../pickboard/pickboardpicks.h \
	    keyboardimpl.h 
SOURCES	=   keyboard.cpp \
	    ../pickboard/pickboardcfg.cpp \
	    ../pickboard/pickboardpicks.cpp \
	    keyboardimpl.cpp
TARGET		= qkeyboard
DESTDIR		= ../../plugins/inputmethods
INCLUDEPATH     += ../../library ../../taskbar
DEPENDPATH      += ../../library ../../taskbar
LIBS            += -lqpe
VERSION		= 1.0.0
