# $Id: citytime.pro,v 1.5 2001/03/16 02:10:33 twschulz Exp $
CONFIG += qt warn_on release
TEMPLATE = app
DESTDIR         = ../bin
INTERFACES = citytimebase.ui
HEADERS = citytime.h zonemap.h sun.h stylusnormalizer.h
SOURCES = citytime.cpp zonemap.cpp main.cpp sun.c stylusnormalizer.cpp
TARGET = citytime
INCLUDEPATH     += ../library
DEPENDPATH      += ../library
LIBS            += -lqpe
