TEMPLATE	= lib
CONFIG		+= qt warn_on release
HEADERS	=   qimpenchar.h \
	    qimpenprofile.h \
	    qimpencombining.h \
	    qimpenhelp.h \
	    qimpeninput.h \
	    qimpenmatch.h \
	    qimpensetup.h \
	    qimpenstroke.h \
	    qimpenwidget.h \
	    qimpenwordpick.h \
	    handwritingimpl.h
SOURCES	=   qimpenchar.cpp \
	    qimpenprofile.cpp \
	    qimpencombining.cpp \
	    qimpenhelp.cpp \
	    qimpeninput.cpp \
	    qimpenmatch.cpp \
	    qimpensetup.cpp \
	    qimpenstroke.cpp \
	    qimpenwidget.cpp \
	    qimpenwordpick.cpp \
	    handwritingimpl.cpp
INTERFACES = qimpenprefbase.ui
TARGET		= qhandwriting
DESTDIR		= ../../plugins/inputmethods
INCLUDEPATH     += ../../library ../../taskbar
DEPENDPATH      += ../../library ../../taskbar
LIBS            += -lqpe
VERSION		= 1.0.0
