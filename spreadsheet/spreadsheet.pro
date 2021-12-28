TEMPLATE    	= app
CONFIG      	= qt warn_on release
DESTDIR		= ../bin

HEADERS     	=  spreadsheet.h \
		   pimspreadsheetwindow.h 

SOURCES     	=  spreadsheet.cpp \
		  pimspreadsheetwindow.cpp \
		  main.cpp

INTERFACES	= spreadsheetwidget.ui

INCLUDEPATH	+= ../library
DEPENDPATH	+= ../library
LIBS            += -lqpe

TARGET      	= spreadsheet
