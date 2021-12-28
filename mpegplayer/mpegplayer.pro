TEMPLATE	= app
CONFIG		= qt warn_on release
DESTDIR		= ../bin
HEADERS		= mpegview.h mpegcontrol.h mediaplayerplugininterface.h libmpeg3plugin.h \
		  audiowidget.h playlistwidget.h mediaplayer.h audiodevice.h
SOURCES		= main.cpp \
		  mpegview.cpp mpegcontrol.cpp libmpeg3plugin.cpp \
		  audiowidget.cpp playlistwidget.cpp mediaplayer.cpp audiodevice.cpp
INTERFACES	= 
TARGET		= mpegplayer
INCLUDEPATH = libmpeg3
#CONFIG+=static
TMAKE_CXXFLAGS += -DQPIM_STANDALONE
INCLUDEPATH	+= ../library
DEPENDPATH	+= ../library
LIBS            += -lqpe
LIBS += libmpeg3/libmpeg3.a -lpthread
