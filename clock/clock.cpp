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

#include <math.h>

#include <qlcdnumber.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qpeapplication.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qpainter.h>

#include "clock.h"

const double deg2rad = 0.017453292519943295769;	// pi/180

Clock::Clock( QWidget * parent, const char * name, WFlags f )
    : QVBox( parent, name , f )
{
    setSpacing( 4 );
    setMargin( 1 );

    aclock = new AnalogClock( this );
    aclock->display( QTime::currentTime() );
    aclock->setLineWidth( 2 );

    lcd = new QLCDNumber( this );
    lcd->setNumDigits( 8 );
    lcd->setSegmentStyle( QLCDNumber::Flat );
    lcd->display( QTime::currentTime().toString() );
    lcd->setFrameStyle( QFrame::NoFrame );

    date = new QLabel( this );
    date->setAlignment( AlignHCenter | AlignVCenter );
    date->setFont( QFont( "Helvetica", 14, QFont::Bold ) );
    date->setText( QDate::currentDate().toString() );

    QWidget *controls = new QWidget( this );
    QGridLayout *gl = new QGridLayout( controls, 2, 2, 6, 4 );

    clockRB = new QRadioButton ( tr( "Clock" ), controls  );
    gl->addWidget( clockRB, 0, 0 );
    swatchRB = new QRadioButton ( tr( "Stopwatch" ), controls );
    gl->addWidget( swatchRB, 1, 0 );
    clockRB->setChecked( TRUE );

    set = new QPushButton ( controls );
    gl->addWidget( set, 0, 1 );
    set->setText( tr( "Start" ) );
    set->setEnabled( FALSE );

    reset = new QPushButton ( controls );
    gl->addWidget( reset, 1, 1 );
    reset->setText( tr( "Reset" ) );
    reset->setEnabled( FALSE );

    connect( clockRB, SIGNAL( clicked() ), SLOT( slotClock() ) );
    connect( swatchRB, SIGNAL( clicked() ), SLOT( slotSWatch() ) );
    connect( set, SIGNAL( clicked() ), SLOT( slotSet() ) );
    connect( reset, SIGNAL( clicked() ), SLOT( slotReset() ) );

    t = new QTimer( this );
    connect( t, SIGNAL( timeout() ), SLOT( updateClock() ) );
    t->start( 1000 );

    connect( qApp, SIGNAL( timeChanged() ), SLOT( updateClock() ) );

    swatch_time = QTime( 0,0,0 );
    swatch_running = FALSE;
}

void Clock::updateClock()
{
    if (swatch_running) {
	swatch_time = swatch_time.addSecs(1);
    }

    if ( clockRB->isChecked() ) {
	lcd->display( QTime::currentTime().toString() );
	aclock->display( QTime::currentTime() );
	date->setText( QDate::currentDate().toString() );
    }
    else {
	lcd->display ( swatch_time.toString() );
	aclock->display( swatch_time );
	date->setText( QDate::currentDate().toString() );

    }
}

void Clock::clearClock( void )
{
    lcd->display( QTime( 0,0,0 ).toString() );
    aclock->display( QTime( 0,0,0 ) );
}

void Clock::slotSet()
{
    if  ( t->isActive() ) {
	set->setText( tr( "Start" ) );
	t->stop();
	swatch_running = FALSE;
    }
    else {
	set->setText( tr( "Stop" ) );
        t->start( 1000 );
	swatch_running = TRUE;
    }
}

void Clock::slotReset()
{
    t->stop();
    clearClock();
    swatch_time.setHMS( 0,0,0 );

    if (! swatch_running )
	updateClock();
    else
	t->start(1000);
}

void Clock::slotClock()
{
    clockRB->setChecked( TRUE );
    swatchRB->setChecked( FALSE );

    set->setEnabled( FALSE );
    reset->setEnabled( FALSE );
    t->start(1000);
    updateClock();
}

void Clock::slotSWatch()
{
    clockRB->setChecked( FALSE );
    swatchRB->setChecked( TRUE );

    set->setEnabled( TRUE );
    reset->setEnabled( TRUE );

    if ( !swatch_running )
	t->stop();
    updateClock();
}

QSizePolicy AnalogClock::sizePolicy() const
{
    return QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
}

void AnalogClock::drawContents( QPainter *p )
{
    QRect r = contentsRect();
    QPoint center( r.x() + r.width() / 2, r.y() + r.height() / 2 );

    QPoint l1( r.x() + r.width() / 2, r.y() + 2 );
    QPoint l2( r.x() + r.width() / 2, r.y() + 8 );

    QPoint h1( r.x() + r.width() / 2, r.y() + r.height() / 4 );
    QPoint h2( r.x() + r.width() / 2, r.y() + r.height() / 2 );

    QPoint m1( r.x() + r.width() / 2, r.y() + r.height() / 8 );
    QPoint m2( r.x() + r.width() / 2, r.y() + r.height() / 2 );

    QPoint s1( r.x() + r.width() / 2, r.y() + 8 );
    QPoint s2( r.x() + r.width() / 2, r.y() + r.height() / 2 );

    QColor color( clear ? backgroundColor() : black );
    QTime time = clear ? prevTime : currTime;

    if ( !clear ) {
	// draw ticks
	p->setPen( QPen( color, 1 ) );
	for ( int i = 0; i < 12; i++ )
	    p->drawLine( rotate( center, l1, i * 30 ), rotate( center, l2, i * 30 ) );
    }

    if ( !clear || prevTime.minute() != currTime.minute() ||
	    prevTime.hour() != currTime.hour() ) {
	// draw hour pointer
	h1 = rotate( center, h1, 30 * ( time.hour() % 12 ) + time.minute() / 2 );
	h2 = rotate( center, h2, 30 * ( time.hour() % 12 ) + time.minute() / 2 );
	p->setPen( QPen( color, 3 ) );
	p->drawLine( h1, h2 );
    }

    if ( !clear || prevTime.minute() != currTime.minute() ) {
	// draw minute pointer
	m1 = rotate( center, m1, time.minute() * 6 );
	m2 = rotate( center, m2, time.minute() * 6 );
	p->setPen( QPen( color, 2 ) );
	p->drawLine( m1, m2 );
    }

    // draw second pointer
    s1 = rotate( center, s1, time.second() * 6 );
    s2 = rotate( center, s2, time.second() * 6 );
    p->setPen( QPen( color, 1 ) );
    p->drawLine( s1, s2 );

    prevTime = currTime;
}

void AnalogClock::display( const QTime& t )
{
    currTime = t;
    clear = true;
    repaint( false );
    clear = false;
    repaint( false );
}

QPoint AnalogClock::rotate( QPoint c, QPoint p, int a )
{
    double angle = deg2rad * ( - a + 180 );
    double nx = c.x() - ( p.x() - c.x() ) * cos( angle ) -
		( p.y() - c.y() ) * sin( angle );
    double ny = c.y() - ( p.y() - c.y() ) * cos( angle ) +
		( p.x() - c.x() ) * sin( angle );
    return QPoint( nx, ny );
}
