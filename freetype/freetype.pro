TEMPLATE	= lib
CONFIG		+= qt warn_on release
HEADERS	=	fontfactoryttf_qws.h freetypefactoryimpl.h
SOURCES	=	fontfactoryttf_qws.cpp freetypefactoryimpl.cpp
qt2:HEADERS	+= qfontdata_p.h
TARGET		= freetypefactory
DESTDIR		= ../plugins/fontfactories
INCLUDEPATH     += ../library
DEPENDPATH      += ../library
#INCLUDEPATH	+= /usr/local/include/freetype2
#DEPENDPATH	+= /usr/local/include/freetype2
LIBS            += -lqpe -lfreetype
VERSION		= 1.0.0
