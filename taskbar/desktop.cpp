/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Palmtop Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#include <qwindowsystem_qws.h>
#include <qmessagebox.h>
#include <qmainwindow.h>
#include <qcopchannel_qws.h>
#include "launcher.h"
#include "shutdownimpl.h"
#include "desktop.h"
#include "info.h"
#include "mrulist.h"
#include "taskbar.h"
#include <stdlib.h>
#include <unistd.h>
#include <global.h>
#ifdef QT_QWS_CUSTOM
#include "custom.h"
#endif

DesktopApplication::DesktopApplication( int& argc, char **argv, Type t )
    : QPEApplication( argc, argv, t )
{
}

#ifdef Q_WS_QWS
bool DesktopApplication::qwsEventFilter( QWSEvent *e )
{
    if ( e->type == QWSEvent::Key ) {
	QWSKeyEvent *ke = (QWSKeyEvent *)e;
	if ( ke->simpleData.is_press ) {
	    if ( !keyboardGrabbed() ) {
		if ( ke->simpleData.keycode == Key_F9 ) {
		    emit datebook();
		    return TRUE;
		}
		if ( ke->simpleData.keycode == Key_F10 ) {
		    emit contacts();
		    return TRUE;
		}
		if ( ke->simpleData.keycode == Key_F11 ) {
		    emit menu();
		    return TRUE;
		}
		if ( ke->simpleData.keycode == Key_F12 ) {
		    emit launch();
		    return TRUE;
		}
		if ( ke->simpleData.keycode == Key_F13 ) {
		    emit email();
		    return TRUE;
		}
	    }
	    if ( ke->simpleData.keycode == Key_F34 ) {
		emit power();
		return TRUE;
	    }
	    if ( ke->simpleData.keycode == Key_F35 ) {
		emit backlight();
		return TRUE;
	    }
	}
    }

    return QPEApplication::qwsEventFilter( e );
}
#endif

Desktop::Desktop() :
    QWidget( 0, 0, WStyle_Tool | WStyle_Customize )
{
//    bg = new Info( this );
    tb = new TaskBar;

    launcher = new Launcher( 0, 0, WStyle_Customize | QWidget::WGroupLeader );

    connect(launcher, SIGNAL(busy()), tb, SLOT(startWait()));
    connect(launcher, SIGNAL(notBusy()), tb, SLOT(stopWait()));

    int displayw = qApp->desktop()->width();
    int displayh = qApp->desktop()->height();

    
    QSize sz = tb->sizeHint();

    setGeometry( 0, displayh-sz.height(), displayw, sz.height() );
    tb->setGeometry( 0, displayh-sz.height(), displayw, sz.height() );

    tb->show();
    launcher->showMaximized();
    launcher->show();
    launcher->raise();
}

Desktop::~Desktop()
{
    delete launcher;
    delete tb;
}

static bool isVisibleWindow(int wid)
{
    const QList<QWSWindow> &list = qwsServer->clientWindows();
    QWSWindow* w;
    for (QListIterator<QWSWindow> it(list); (w=it.current()); ++it) {
	if ( w->winId() == wid )
	    return !w->isFullyObscured();
    }
    return FALSE;
}

static bool isVisibleWindow(const QString& windowname)
{
    const QList<QWSWindow> &list = qwsServer->clientWindows();
    QWSWindow* w;
    for (QListIterator<QWSWindow> it(list); (w=it.current()); ++it) {
	if ( w->name() == windowname && !w->isFullyObscured() )
	    return TRUE;
    }
    return FALSE;
}

void Desktop::raiseLauncher()
{
    if ( isVisibleWindow(launcher->winId()) )
	launcher->nextView();
    else
	launcher->raise();
}

void Desktop::executeOrModify(const QCString& app)
{
    if ( QCopChannel::isRegistered("QPE/Application/" + app) )
	if ( isVisibleWindow(app) )
	    QCopChannel::send("QPE/Application/" + app, "nextView()");
	else
	    QCopChannel::send("QPE/Application/" + app, "raise()");
    else
	Global::execute(app);
}

void Desktop::raiseDatebook()
{
    executeOrModify("datebook");
}

void Desktop::raiseContacts()
{
    executeOrModify("addressbook");
}

void Desktop::raiseMenu()
{
    executeOrModify("todo");
}

void Desktop::raiseEmail()
{
    executeOrModify("qtmail");
}

#if defined(QPE_HAVE_TOGGLELIGHT)
#include <config.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <linux/ioctl.h>
#include <time.h>
#endif

void Desktop::togglePower()
{
    system("apm --suspend");
}

void Desktop::toggleLight()
{
#if defined(HAVE_TOGGLELIGHT)
#define FL_IOCTL_STEP_CONTRAST    100
#define FL_IOCTL_GET_STEP_CONTRAST 101
    int fd = open("/dev/fl", O_WRONLY);
    if (fd >= 0 ) {
	Config config( "qpe" );
	config.setGroup( "Screensaver" );
	int bright = config.readNumEntry("Brightness",128);
        bright = bright * 5 / 255;
	if (bright > 4)
            bright = 4;
	int ret = ioctl(fd, FL_IOCTL_GET_STEP_CONTRAST, 0);
	if (ret != bright)
            ioctl(fd, FL_IOCTL_STEP_CONTRAST, bright);
        else
            ioctl(fd, FL_IOCTL_STEP_CONTRAST, 0);
    }
#endif
}

void Desktop::styleChange( QStyle &s )
{
    QWidget::styleChange( s );
    int displayw = qApp->desktop()->width();
    int displayh = qApp->desktop()->height();

    QSize sz = tb->sizeHint();

    tb->setGeometry( 0, displayh-sz.height(), displayw, sz.height() );
}

void DesktopApplication::shutdown()
{
    if ( type() != GuiServer )
	return;
    ShutdownImpl *sd = new ShutdownImpl( 0, 0, WDestructiveClose );
    connect( sd, SIGNAL(shutdown(ShutdownImpl::Type)),
	     this, SLOT(shutdown(ShutdownImpl::Type)) );
    sd->showMaximized();
}

void DesktopApplication::shutdown( ShutdownImpl::Type t )
{
    switch ( t ) {
	case ShutdownImpl::ShutdownSystem:
	    execlp("shutdown", "shutdown", "-h", "now");
	    break;
	case ShutdownImpl::RebootSystem:
	    execlp("shutdown", "shutdown", "-r", "now");
	    break;
	case ShutdownImpl::RestartDesktop:
	    restart();
	    break;
	case ShutdownImpl::TerminateDesktop:
	    prepareForTermination(FALSE);
	    quit();
	    break;
    }
}

void DesktopApplication::restart()
{
    prepareForTermination(TRUE);

#ifdef Q_WS_QWS
    for ( int fd = 3; fd < 100; fd++ )
	close( fd );
#if defined(QT_DEMO_SINGLE_FLOPPY)
    execl( "/sbin/init", "qpe", 0 );
#elif defined(QT_QWS_CASSIOPEIA)
    execl( "/bin/sh", "sh", 0 );
#else
    execl( (qpeDir()+"/bin/qpe").latin1(), "qpe", 0 );
#endif
    exit(1);
#endif
}

