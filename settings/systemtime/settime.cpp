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
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qtoolbutton.h>
#include <qcheckbox.h>

#include <config.h>
#include <tzselect.h>
#include <resource.h>
#include <global.h>

#if ( defined Q_WS_QWS || defined(_WS_QWS_) ) && !defined(QT_NO_COP)
#include "qcopenvelope_qws.h"
#endif

#include "datebookmonth.h"
#include "settime.h"


SetDateTime::SetDateTime(QWidget *parent, const char *name, bool modal,  WFlags f )
    : QDialog( parent, name, modal, f )
{
    setCaption( tr("Set System Time") );

    QVBoxLayout *vb = new QVBoxLayout( this, 5 );

    QHBoxLayout *hb = new QHBoxLayout( vb, -1, "timezone layout" );

    QLabel *lblZone = new QLabel( tr( "Time Zone" ), this, "timezone label" );
    lblZone->setMaximumSize( lblZone->sizeHint() );
    hb->addWidget( lblZone );

    tz = new TimeZoneSelector( this, "Timezone choices" );
    tz->setMinimumSize( tz->sizeHint() );
    hb->addWidget( tz );

    //QLabel *l = new QLabel( tr("<b>Time</b>"), this );
    //vb->addWidget( l );

    time = new SetTime( this );
    vb->addWidget( time );

    
    QFrame *f = new QFrame( this );
    f->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    vb->addWidget( f );

    //l = new QLabel( tr("<b>Date</b>"), this );
    //vb->addWidget( l );

    date = new SetDate( this );
    vb->addWidget( date );

    QObject::connect( tz, SIGNAL( signalNewTz( const QString& ) ),
                      time, SLOT( slotTzChange( const QString& ) ) );
    QObject::connect( tz, SIGNAL( signalNewTz( const QString& ) ),
                      date, SLOT( slotTzChange( const QString& ) ) );
}

void SetDateTime::accept()
{
    Config config("qpe");
    config.setGroup( "Time" );
    config.writeEntry( "AMPM", time->viewAP() );

    Config lconfig("locale");
    lconfig.setGroup( "Location" );
    lconfig.writeEntry( "Timezone", tz->currentZone() );
  
    // before we progress further, set our TZ!
    setenv( "TZ", tz->currentZone(), 1 );
    // now set the time...
    QDateTime dt( date->date(), time->time() );
    // stime() is Linux-centric, the more generic way...
    int change = QDateTime::currentDateTime().secsTo( dt );
    struct timeval myTv;
    ::gettimeofday( &myTv, 0 );
    myTv.tv_sec += change;
    ::settimeofday( &myTv, 0 ); 
    system("/sbin/hwclock --systohc"); // ##### UTC?

    // set the timezone for everyone else...
    QCopEnvelope e( "QPE/System", "timeChange(QString)" );
    e << tz->currentZone();

    QCopEnvelope anotherEnv( "QPE/System", "clockChange(bool)" );
    anotherEnv << time->viewAP();

    QDialog::accept();
}

SetTime::SetTime( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    Config config( "qpe" );

    config.setGroup( "Time" );

    QTime currTime = QTime::currentTime();
    hour = currTime.hour();
    minute = currTime.minute();

    QHBoxLayout *hb = new QHBoxLayout( this );

    QLabel *l = new QLabel( tr("AM/PM"), this );
    l->setAlignment( AlignRight | AlignVCenter );
    hb->addWidget( l );
    ampm = new QCheckBox( this );
    ampm->setChecked( config.readBoolEntry( "AMPM" ) );
    hb->addWidget( ampm );

    l = new QLabel( tr("Hour"), this );
    l->setAlignment( AlignRight | AlignVCenter );
    hb->addWidget( l );

    sbHour = new QSpinBox( this );
    sbHour->setMinValue( 0 );
    sbHour->setMaxValue( 23 );
    sbHour->setWrapping(TRUE);
    sbHour->setValue( hour );
    connect( sbHour, SIGNAL(valueChanged(int)), this, SLOT(hourChanged(int)) );
    hb->addWidget( sbHour );

    l = new QLabel( tr("Minute"), this );
    l->setAlignment( AlignRight | AlignVCenter );
    hb->addWidget( l );

    sbMin = new QSpinBox( this );
    sbMin->setMinValue( 0 );
    sbMin->setMaxValue( 59 );
    sbMin->setWrapping(TRUE);
    sbMin->setValue( minute );
    connect( sbMin, SIGNAL(valueChanged(int)), this, SLOT(minuteChanged(int)) );
    hb->addWidget( sbMin );
}

QTime SetTime::time() const
{
    return QTime( hour, minute, 0 );
}

void SetTime::hourChanged( int value )
{
    hour = value;
}

void SetTime::minuteChanged( int value )
{
    minute = value;
}

void SetTime::slotTzChange( const QString &tz )
{
    // set the TZ get the time and leave gracefully...
    QString strSave;
    strSave = getenv( "TZ" );
    setenv( "TZ", tz, 1 );

    QTime t = QTime::currentTime();
    // reset the time.
    if ( !strSave.isNull() ) {
	setenv( "TZ", strSave, 1 );
    }
    
    // just set the spinboxes and let it propage through
    sbHour->setValue( t.hour() );
    sbMin->setValue( t.minute() );
}

bool SetTime::viewAP( void ) const {
  return ampm ? ampm->isChecked() : false; 
};


SetDate::SetDate( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    m_date = QDate::currentDate();

    QHBoxLayout *hb = new QHBoxLayout( this );

    dbm = new DateBookMonth( this );
    
    connect( dbm, SIGNAL(dateClicked(int,int,int)),
	     this, SLOT(dateClicked(int,int,int)) );
    hb->addWidget( dbm );
}

void SetDate::dateClicked( int year, int month, int day )
{
    m_date.setYMD( year, month, day );
}

void SetDate::slotTzChange( const QString &tz )
{
    // set the TZ get the time and leave gracefully...
    QString strSave;
    strSave = getenv( "TZ" );
    setenv( "TZ", tz, 1 );

    QDate d = QDate::currentDate();
    // reset the time.
    if ( !strSave.isNull() ) {
	setenv( "TZ", strSave, 1 );
    }
    dbm->setDate( d.year(), d.month(), d.day() );
}

