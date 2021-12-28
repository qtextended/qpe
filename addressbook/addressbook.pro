TEMPLATE	= app
CONFIG		= qt warn_on release
DESTDIR		= ../bin
HEADERS	= addressbook.h \
                  abeditor.h \
		  abeditorpage2.h \
		  abentry.h \
		  ablabel.h \
		  abtable.h \
		  addresssettings.h
SOURCES	= main.cpp \
		  addressbook.cpp \
		  abeditor.cpp \
		  abeditorpage2.cpp \
		  abentry.cpp \
		  ablabel.cpp \
		  abtable.cpp \
		  addresssettings.cpp
INTERFACES	= abeditorbase.ui abeditorpage2base.ui addresssettingsbase.ui

TARGET		= addressbook
INCLUDEPATH	+= ../library
DEPENDPATH	+= ../library
LIBS            += -lqpe
