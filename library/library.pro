TEMPLATE	= lib
CONFIG		+= qt warn_on release
HEADERS	=   calendar.h \
	    global.h \
	    organizer.h \
	    resource.h \
	    xmlreader.h \
	    mimetype.h \
	    menubutton.h \
	    filemanager.h \
	    fontmanager.h \
	    qdawg.h \
	    datebookmonth.h \
	    fileselector.h \
	    imageedit.h \
	    qcopenvelope_qws.h \
	    qpedecoration_qws.h \
	    qpeapplication.h \
	    qpestyle.h \
	    lightstyle.h \
	    config.h \
	    applnk.h \
	    sound.h \
	    tzselect.h \
	    qmath.h \
	    datebookdb.h \
	    alarmserver.h \
	    process.h \
	    timestring.h \
	    fontfactoryinterface.h \
	    fontdatabase.h \
	    qpedebug.h
SOURCES	=   calendar.cpp \
	    global.cpp \
	    organizer.cpp \
	    xmlreader.cpp \
	    mimetype.cpp \
	    menubutton.cpp \
	    filemanager.cpp \
	    fontmanager.cpp \
	    qdawg.cpp \
	    datebookmonth.cpp \
	    fileselector.cpp \
	    imageedit.cpp \
	    resource.cpp \
	    qpedecoration_qws.cpp \
	    qcopenvelope_qws.cpp \
	    qpeapplication.cpp \
	    qpestyle.cpp \
	    lightstyle.cpp \
	    config.cpp \
	    applnk.cpp \
	    sound.cpp \
	    tzselect.cpp \
	    qmath.c \
	    datebookdb.cpp \
	    alarmserver.cpp \
	    process.cpp \
	    process_unix.cpp \
	    timestring.cpp \
	    fontdatabase.cpp \
	    qpedebug.cpp

# Qt 3 compatibility
qt2:HEADERS += quuid.h qcom.h qlibrary.h qlibrary_p.h
qt2:SOURCES += quuid.cpp qlibrary.cpp qlibrary_unix.cpp

INCLUDEPATH	+= .
LIBS		+= -ldl

TARGET		= qpe
DESTDIR		= $(QTDIR)/lib$(PROJMAK)
# This is set by configure...
VERSION		= 1.3.1
