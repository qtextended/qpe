TEMPLATE	= app
CONFIG		+= qt warn_on release
DESTDIR		= ../bin

HEADERS	= datebookday.h \
		  datebook.h \
		  dateentryimpl.h \
		  datebookdayheaderimpl.h \
		  datebooksettings.h \
		  datebookweek.h \
		  datebookweekheaderimpl.h \
		  repeatentry.h

SOURCES	= main.cpp \
		  datebookday.cpp \
		  datebook.cpp \
		  dateentryimpl.cpp \
		  datebookdayheaderimpl.cpp \
		  datebooksettings.cpp \
		  datebookweek.cpp \
		  datebookweekheaderimpl.cpp \
		  repeatentry.cpp

INTERFACES	= dateentry.ui \
		  datebookdayheader.ui \
		  datebooksettingsbase.ui \
		  datebookweekheader.ui \
		  repeatentrybase.ui

INCLUDEPATH	+= ../library
DEPENDPATH	+= ../library
LIBS            += -lqpe

TARGET		= datebook
