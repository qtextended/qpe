TEMPLATE        = app
CONFIG          += qt warn_on release
DESTDIR         = ../bin
HEADERS		= mindbreaker.h
SOURCES		= main.cpp \
              mindbreaker.cpp
INTERFACES  = helpdialog.ui
TARGET          = mindbreaker
INCLUDEPATH     += ../library
DEPENDPATH      += ../library
LIBS            += -lqpe
