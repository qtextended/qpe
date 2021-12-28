TEMPLATE	= app

CONFIG		+= qt warn_on release

DESTDIR		= ../bin

HEADERS		= TEWidget.h \
		  TEScreen.h \
		  TECommon.h \
		  TEHistory.h \
		  TEmulation.h \
		  TEmuVt102.h \
		  session.h \
		  keytrans.h \
		  konsole.h \
		  MyPty.h

SOURCES		= TEScreen.cpp \
		  TEWidget.cpp \
		  TEHistory.cpp \
		  TEmulation.cpp \
		  TEmuVt102.cpp \
		  session.cpp \
		  keytrans.cpp \
		  konsole.cpp \
		  main.cpp \
		  MyPty.cpp

TARGET		= embeddedkonsole

INCLUDEPATH	+= ../library

DEPENDPATH	+= ../library

LIBS            += -lqpe -lutil

REQUIRES	= embeddedkonsole

