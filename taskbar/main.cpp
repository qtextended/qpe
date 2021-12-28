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
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <qpeapplication.h>
#include <config.h>
#include <qfile.h>
#include <qwindowsystem_qws.h>
#include <qcopenvelope_qws.h>
#include "desktop.h"
#include "taskbar.h"
#include "transferserver.h"
#ifdef QT_QWS_CUSTOM
#include "custom.h"
#endif

#ifdef QT_QWS_LOGIN
#include "../login/qdmdialogimpl.h"
#endif

#ifdef QT_QWS_CASSIOPEIA
static void ignoreMessage( QtMsgType, const char * )
{
}
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <qdatetime.h>

void initCassiopeia()
{
    // MIPSEL-specific init - make sure /proc exists for shm
/*
    if ( mount("/dev/ram0", "/", "ext2", MS_REMOUNT | MS_MGC_VAL, 0 ) ) {
	perror("Remounting - / read/write");
    }
*/
    if ( mount("none", "/tmp", "ramfs", 0, 0 ) ) {
	perror("mounting ramfs /tmp");
    } else {
	fprintf( stderr, "mounted /tmp\n" );
    }
    if ( mount("none", "/home", "ramfs", 0, 0 ) ) {
	perror("mounting ramfs /home");
    } else {
	fprintf( stderr, "mounted /home\n" );
    }
    if ( mount("none","/proc","proc",0,0) ) {
	perror("Mounting - /proc");
    } else {
	fprintf( stderr, "mounted /proc\n" );
    }
    if ( mount("none","/mnt","shm",0,0) ) {
	perror("Mounting - shm");
    }
    setenv( "QTDIR", "/", 1 );
    setenv( "QPEDIR", "/", 1 );
    setenv( "HOME", "/home", 1 );
    mkdir( "/home/Documents", 0755 );

    // set a reasonable starting date
    QDateTime dt( QDate( 2001, 3, 15 ) );
    QDateTime now = QDateTime::currentDateTime();
    int change = now.secsTo( dt );

    time_t t = ::time(0);
    t += change;
    stime(&t);

    qInstallMsgHandler(ignoreMessage);
}
#endif

#ifdef QPE_OWNAPM
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <linux/ioctl.h>
#include <asm/sharp_apm.h>
#include <global.h>

static void disableAPM() 
{

    int fd, cur_val, ret;
    char *device = "/dev/apm_bios";

    fd = open (device, O_WRONLY);

    if (fd ==  -1) {
      perror(device);
      return;
    }

    cur_val = ioctl(fd, APM_IOCGEVTSRC, 0);
    if (cur_val == -1) {
      perror("ioctl");
      exit(errno);
    }

    ret = ioctl(fd, APM_IOCSEVTSRC, cur_val & ~APM_EVT_POWER_BUTTON);
    if (ret == -1) {
        perror("ioctl");
        return;
    }
    close(fd);
}

static void initAPM()
{
    // So that we have to do it ourself, but better.
    disableAPM();
}
#endif

#ifdef QT_DEMO_SINGLE_FLOPPY
#include <sys/mount.h>

void initFloppy()
{
    mount("none","/proc","proc",0,0);
    setenv( "QTDIR", "/", 0 );
    setenv( "HOME", "/root", 0 );
    setenv( "QWS_SIZE", "240x320", 0 );
}
#endif


void initEnvironment()
{
  Config config("locale");
  config.setGroup( "Location" );
  QString tz = config.readEntry( "Timezone", getenv("TZ") );
  if ( !tz.isNull() )
    setenv( "TZ", tz, 1 );

  config.setGroup( "Language" );
  QString lang = config.readEntry( "Language", getenv("LANG") );
  if ( !lang.isNull() )
    setenv( "LANG", lang, 1 );
}

static void initBacklight()
{
    QCopEnvelope e("QPE/System", "setBacklight(int)" );
    e << -1;
}



int initApplication( int argc, char ** argv )
{
#ifdef QT_QWS_CASSIOPEIA
    initCassiopeia();
#endif

#ifdef QPE_OWNAPM
    initAPM();
#endif

#ifdef QT_DEMO_SINGLE_FLOPPY
    initFloppy();
#endif

    initEnvironment();

#if !defined(QT_QWS_CASSIOPEIA) && !defined(QT_QWS_IPAQ) && !defined(QT_QWS_EBX)
    setenv( "QWS_SIZE", "240x320", 0 );
#endif

    //Don't flicker at startup:
    QWSServer::setDesktopBackground( QImage() );
    DesktopApplication a( argc, argv, QApplication::GuiServer );

    initBacklight();

    QWidget *cal = 0;

#if defined(QT_QWS_CASSIOPEIA) || defined(QT_QWS_IPAQ) || defined(QT_QWS_EBX)
    if ( !QFile::exists( "/etc/pointercal" ) ) {
	cal = TaskBar::calibrate(TRUE);
    }
#endif

#if defined(QT_QWS_LOGIN)
    for( int i=0; i<a.argc(); i++ )
      if( strcmp( a.argv()[i], "-login" ) == 0 ) {
	QDMDialogImpl::login( );
	return 0;
      }
#endif

    Desktop *d = new Desktop();
    d->show();

    QObject::connect( &a, SIGNAL(datebook()), d, SLOT(raiseDatebook()) );
    QObject::connect( &a, SIGNAL(contacts()), d, SLOT(raiseContacts()) );
    QObject::connect( &a, SIGNAL(menu()),  d, SLOT(raiseMenu()) );
    QObject::connect( &a, SIGNAL(launch()),   d, SLOT(raiseLauncher()) );
    QObject::connect( &a, SIGNAL(email()),   d, SLOT(raiseEmail()) );
    QObject::connect( &a, SIGNAL(power()),   d, SLOT(togglePower()) );
    QObject::connect( &a, SIGNAL(backlight()),   d, SLOT(toggleLight()) );

    // Make sure calibration widget starts on top.
    if ( cal )
	cal->raise();


    // start transfer server
    (void) new TransferServer( 4242 );

    int rv =  a.exec();

    delete d;

    return rv;
}

int main( int argc, char ** argv )
{
#ifndef SINGLE_APP
    signal( SIGCHLD, SIG_IGN );
#endif

    int retVal = initApplication( argc, argv );

#ifndef SINGLE_APP
    // Kill them. Kill them all.
    setpgid( getpid(), getppid() );
    killpg( getpid(), SIGTERM );
    sleep( 1 );
    killpg( getpid(), SIGKILL );
#endif

    return retVal;
}

