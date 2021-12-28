TEMPLATE	= app
CONFIG		+= qt warn_on release

DESTDIR		= ../bin

HEADERS	= textedit.h

SOURCES	= main.cpp textedit.cpp

INCLUDEPATH	+= ../library
DEPENDPATH	+= ../library
LIBS            += -lqpe

TARGET		= textedit
