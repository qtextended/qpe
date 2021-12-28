TEMPLATE	= app

CONFIG		= qt warn_on release

DESTDIR		= ../bin

HEADERS		= background.h \
		  desktop.h \
		  info.h \
		  appicons.h \
		  taskbar.h \
		  sidething.h \
		  mrulist.h \
		  inputmethods.h \
		  systray.h \
		  wait.h \
		  shutdownimpl.h \
		  ../launcher/launcher.h \
		  ../launcher/launcherview.h \
		  ../launcher/lnkproperties.h \
		  ../launcher/categorylist.h \
		  ../calibrate/calibrate.h \
		  transferserver.h

#		  startmenu.h \
#		  quicklauncher.h \

SOURCES		= background.cpp \
		  desktop.cpp \
		  info.cpp \
		  appicons.cpp \
		  taskbar.cpp \
		  sidething.cpp \
		  mrulist.cpp \
		  inputmethods.cpp \
		  systray.cpp \
		  wait.cpp \
		  shutdownimpl.cpp \
		  ../launcher/launcher.cpp \
		  ../launcher/launcherview.cpp \
		  ../launcher/lnkproperties.cpp \
		  ../launcher/categorylist.cpp \
		  ../calibrate/calibrate.cpp \
		  transferserver.cpp \
		  main.cpp

#		  startmenu.cpp \
#		  quicklauncher.cpp \

INCLUDEPATH	+= ../library .
DEPENDPATH	+= ../library .

INCLUDEPATH	+= ../launcher
DEPENDPATH	+= ../launcher

INCLUDEPATH	+= ../calibrate
DEPENDPATH	+= ../calibrate

TARGET		= qpe

INTERFACES	= shutdown.ui \
		    ../launcher/lnkpropertiesbase.ui

LIBS		+= -lqpe -lcrypt
