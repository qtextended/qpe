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
** $Id: datebook.cpp,v 1.56 2001/09/05 16:02:18 twschulz Exp $
**
**********************************************************************/

#include <qaction.h>
#include <qapplication.h>
#include <qdialog.h>
#include <qfile.h>
#include <qlayout.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qtextcodec.h>
#include <qtextstream.h>
#include <qtoolbar.h>
#include <qwidgetstack.h>
#include <qmessagebox.h>
#include <qsound.h>

#include <config.h>
#include <organizer.h>
#include <resource.h>
#include <xmlreader.h>
#include <qpedebug.h>

#include <cstdlib>

#include "datebook.h"
#include "datebookday.h"
#include <datebookmonth.h>
#include "datebookweek.h"
#include "dateentryimpl.h"
#include "datebooksettings.h"


#define DAY 1
#define WEEK 2
#define MONTH 3


static int make_uid();

DateBook::DateBook( QWidget *parent, const char *, WFlags f )
    : QMainWindow( parent, "datebook", f ),
    aPreset( FALSE ),
    presetTime( -1 ),
    startTime( 8 ) // an acceptable default
{
    db = new DateBookDB;
    gui = TRUE;
    loadSettings();
    setCaption( tr("Date Book") );
    setIcon( Resource::loadPixmap( "datebook_icon" ) );

    setToolBarsMovable( FALSE );

    QToolBar *bar = new QToolBar( this );
    bar->setHorizontalStretchable( TRUE );

    QMenuBar *mb = new QMenuBar( bar );
    mb->setMargin( 0 );

    QPopupMenu *view = new QPopupMenu( this );
    QPopupMenu *settings = new QPopupMenu( this );

    mb->insertItem( tr( "View" ), view );
    mb->insertItem( tr( "Settings" ), settings );

//    bar = new QToolBar( this );

    QAction *a = new QAction( tr( "New" ), Resource::loadPixmap( "new" ), QString::null, 0, this, 0 );
    connect( a, SIGNAL( activated() ), this, SLOT( fileNew() ) );
    a->addTo( bar );

//    bar = new QToolBar( this );

    QActionGroup *g = new QActionGroup( this );
    g->setExclusive( TRUE );

    a = new QAction( tr( "Day" ), Resource::loadPixmap( "day" ), QString::null, 0, g, 0 );
    connect( a, SIGNAL( activated() ), this, SLOT( viewDay() ) );
    a->addTo( bar );
    a->addTo( view );
    a->setToggleAction( TRUE );
    a->setOn( TRUE );
    dayAction = a;
    a = new QAction( tr( "Week" ), Resource::loadPixmap( "week" ), QString::null, 0, g, 0 );
    connect( a, SIGNAL( activated() ), this, SLOT( viewWeek() ) );
    a->addTo( bar );
    a->addTo( view );
    a->setToggleAction( TRUE );
    weekAction = a;
    a = new QAction( tr( "Month" ), Resource::loadPixmap( "month" ), QString::null, 0, g, 0 );
    connect( a, SIGNAL( activated() ), this, SLOT( viewMonth() ) );
    a->addTo( bar );
    a->addTo( view );
    a->setToggleAction( TRUE );
    monthAction = a;

    a = new QAction( tr( "Today" ), QString::null, 0, 0 );
    connect( a, SIGNAL( activated() ), this, SLOT( slotToday() ) );
    a->addTo( view );

    a = new QAction( tr( "Alarm and Start Time..." ), QString::null, 0, 0 );
    connect( a, SIGNAL( activated() ), this, SLOT( slotSettings() ) );
    a->addTo( settings );

    views = new QWidgetStack( this );
    setCentralWidget( views );

    dayView = 0;
    weekView = 0;
    monthView = 0;

    viewDay();

    connect( qApp, SIGNAL(clockChanged(bool)),
             this, SLOT(changeClock(bool)) );

    connect( qApp, SIGNAL(appMessage(const QCString&, const QByteArray&)),
	     this, SLOT(appMessage(const QCString&, const QByteArray&)) );
}

DateBook::~DateBook()
{
    saveSettings();
    saveRequired();
}

void DateBook::saveRequired()
{
    // #### Could start a timer, etc.
    db->save();
}

void DateBook::slotSettings()
{
    DateBookSettings frmSettings( ampm, this );
    frmSettings.setStartTime( startTime );
    frmSettings.setAlarmPreset( aPreset, presetTime );
#if defined (Q_WS_QWS) || defined(_WS_QWS_)
    frmSettings.showMaximized();
#endif

    if ( frmSettings.exec() ) {
	aPreset = frmSettings.alarmPreset();
	presetTime = frmSettings.presetTime();
	startTime = frmSettings.startTime();
	if ( dayView )
	    dayView->setStartViewTime( startTime );
	if ( weekView )
	    weekView->setStartViewTime( startTime );
	saveSettings();
	
	// make the change obvious
	if ( views->visibleWidget() ) {
	    if ( views->visibleWidget() == dayView )
		dayView->redraw();
	    else if ( views->visibleWidget() == weekView )
		weekView->redraw();
	}
    }
}

void DateBook::fileNew()
{
    QDateTime start, end;
    if ( views->visibleWidget() == dayView ) {
        dayView->selectedDates( start, end );
    } else if ( views->visibleWidget() == monthView ) {
        QDate d = monthView->selectedDate();
        start = end = d;
        start.setTime( QTime( 10, 0 ) );
        end.setTime( QTime( 12, 0 ) );
    } else if ( views->visibleWidget() == weekView ) {
	QDate d = weekView->date();
	start = end = d;
	start.setTime( QTime( 10, 0 ) );
	end.setTime( QTime( 12, 0 ) );
    }
    QDialog newDlg( this, 0, TRUE );
    newDlg.setCaption( DateEntryBase::tr("New Event") );
    DateEntry *e;
    QVBoxLayout *vb = new QVBoxLayout( &newDlg );
    QScrollView *sv = new QScrollView( &newDlg, "scrollview" );
    sv->setResizePolicy( QScrollView::AutoOneFit );
    // KLUDGE!!!
    sv->setHScrollBarMode( QScrollView::AlwaysOff );
    vb->addWidget( sv );
    e = new DateEntry( start, end, ampm, &newDlg, "editor" );
    e->setAlarmEnabled( aPreset, presetTime, Event::Loud );
    sv->addChild( e );

#if defined(Q_WS_QWS) || defined(_WS_QWS_)
    newDlg.showMaximized();
#endif
    if ( newDlg.exec() ) {
	// assign a semi-unique id (UUID would be good)
	// for now a -time_t on the IPaq side...
	int uid = make_uid();
	Event newEv = e->event();
	newEv.setUid( uid );
        db->addEvent( newEv );
        emit newEvent();
    }
}

int DateBook::calcWeek( const QDate &d ) const
{
    return ( ( d.dayOfYear() + QDate( d.year(), 1, 1 ).dayOfWeek() - 2 ) / 7 ) + 1;
}

void DateBook::viewDay()
{
    initDay();
    dayAction->setOn( TRUE );
    if ( weekView && views->visibleWidget() == weekView ) {
        QDate d = weekView->date();
	int week = calcWeek( d );
	int oldWeek = calcWeek( dayView->date() );
	if ( week != oldWeek || d.year() != dayView->date().year() )
	    dayView->setDate( d.year(), d.month(), d.day() );
    }
    views->raiseWidget( dayView );
    dayView->redraw();
}

void DateBook::viewWeek()
{
    initWeek();
    weekAction->setOn( TRUE );
    if ( dayView && views->visibleWidget() == dayView ) {
        QDate d = dayView->date();
        int week = calcWeek( d );
	if ( week != weekView->week() )
	    weekView->setDate( d.year(), week );
    }
    views->raiseWidget( weekView );
    weekView->redraw();
}

void DateBook::viewMonth()
{
    initMonth();
    monthAction->setOn( TRUE );
    if ( views->visibleWidget() ) {
	if ( views->visibleWidget() == weekView ) {
	    QDate d = weekView->date();
	    monthView->setDate( d.year(), d.month(), d.day() );
	} else if ( views->visibleWidget() == dayView ) {
	    QDate d = dayView->date();
	    monthView->setDate( d.year(), d.month(), d.day() );
	}
    }
    views->raiseWidget( monthView );
}

void DateBook::editEvent( const Event &e )
{
    // workaround added for text input.
    QDialog editDlg( this, 0, TRUE );
    DateEntry *entry;
    editDlg.setCaption( tr("Edit Event") );
    QVBoxLayout *vb = new QVBoxLayout( &editDlg );
    QScrollView *sv = new QScrollView( &editDlg, "scrollview" );
    sv->setResizePolicy( QScrollView::AutoOneFit );
    // KLUDGE!!!
    sv->setHScrollBarMode( QScrollView::AlwaysOff );
    vb->addWidget( sv );
    entry = new DateEntry( e, ampm, &editDlg, "editor" );
    sv->addChild( entry );

#ifdef _WS_QWS_
    editDlg.showMaximized();
#endif
    if ( editDlg.exec() ) {
	Event newEv = entry->event();
        db->editEvent( e, newEv );
        emit newEvent();
    }
}

void DateBook::removeEvent( const Event &e )
{
    db->removeEvent( e );
    if ( views->visibleWidget() == dayView && dayView )
        dayView->redraw();
}

void DateBook::addEvent( const Event &e )
{
    QDate d = e.start().date();
    initDay();
    dayView->setDate( d.year(), d.month(), d.day() );
}

void DateBook::showDay( int year, int month, int day )
{
    initDay();
    dayView->setDate( year, month, day );
    views->raiseWidget( dayView );
    dayAction->setOn( TRUE );
}

void DateBook::initDay()
{
    if ( !dayView ) {
	dayView = new DateBookDay( ampm, db, views, "day view" );
	views->addWidget( dayView, DAY );
	dayView->setStartViewTime( startTime );
	connect( this, SIGNAL( newEvent() ),
		 dayView, SLOT( redraw() ) );
	connect( dayView, SIGNAL( newEvent() ),
		 this, SLOT( fileNew() ) );
	connect( dayView, SIGNAL( removeEvent( const Event & ) ),
		 this, SLOT( removeEvent( const Event & ) ) );
	connect( dayView, SIGNAL( editEvent( const Event & ) ),
		 this, SLOT( editEvent( const Event & ) ) );
    }
}

void DateBook::initWeek()
{
    if ( !weekView ) {
	weekView = new DateBookWeek( ampm, db, views, "week view" );
	weekView->setStartViewTime( startTime );
	views->addWidget( weekView, WEEK );
	connect( weekView, SIGNAL( showDate( int, int, int ) ),
             this, SLOT( showDay( int, int, int ) ) );
    }
}

void DateBook::initMonth()
{
    if ( !monthView ) {
	monthView = new DateBookMonth( views, "month view", FALSE, db );
	views->addWidget( monthView, MONTH );
	connect( monthView, SIGNAL( dateClicked( int, int, int ) ),
             this, SLOT( showDay( int, int, int ) ) );
    }
}

void DateBook::loadSettings()
{
    {
	Config config( "qpe" );
	config.setGroup("Time");
	ampm = config.readBoolEntry( "AMPM");
    }

    {
	Config config("DateBook");
	config.setGroup("Main");
	startTime = config.readNumEntry("startviewtime");
	aPreset = config.readBoolEntry("alarmpreset");
	presetTime = config.readNumEntry("presettime");
    }
}

void DateBook::saveSettings()
{
    {
	Config config( "qpe" );
	config.setGroup("Time");
	config.writeEntry( "AMPM",ampm);
    }

    {
	Config config( "DateBook" );
	config.setGroup( "Main" );
	config.writeEntry("startviewtime",startTime);
	config.writeEntry("alarmpreset",aPreset);
	config.writeEntry("presettime",presetTime);
    }
}

void DateBook::appMessage(const QCString& msg, const QByteArray& data)
{
    if ( msg == "alarm(QDateTime,int)" ) {
	QDataStream ds(data,IO_ReadOnly);
	QDateTime when; int warn;
	ds >> when >> warn;
	//qDebug("alarm at %s, warn=%d",when.toString().latin1(),warn);
	QValueList<Event> list = db->getEvents(when.addSecs(warn*60));
	if ( list.count() ) {
	    QString msg;
	    bool bSound = FALSE;
	    int stopTimer = 0;
	    bool found = FALSE;
	    for (QValueList<Event>::ConstIterator it=list.begin(); it!=list.end(); ++it) {
		if ( (*it).hasAlarm() ) {
		    found = TRUE;
		    msg += "<CENTER><B>" + (*it).description() + "</B>" + "<BR>"
			   + (*it).location() + "<BR>"
			   + (*it).start().toString()
			   + (warn
			      ? tr("(in " + QString::number(warn)+tr(" minutes)"))
			      : QString(""))
			   + "<BR>"
			   + (*it).notes() + "</CENTER>";
		    if ( (*it).alarmSound() != Event::Silent ) {
			qDebug("event %s has alarm", (*it).description().latin1() );
			bSound = TRUE;
		    }
		}
	    }
	    if ( found ) {
		if ( bSound ) {
		    QSound::play(Resource::findSound("alarm"));
		    stopTimer = startTimer( 5000 );
		}
		if ( isVisible() ) {
		    QMessageBox::information(this, tr("Alarm"), msg,
					     tr("OK") );
		} else {
		    gui = QMessageBox::information(this, tr("Alarm"), msg,
						   tr("OK"), tr("Show..."));
		}
		if ( bSound )
		    killTimer( stopTimer );
	    }
	} else {
	    // out-of-date alarm
	    if ( !isVisible() ) gui=FALSE;
	}
    } else if ( msg == "nextView()" ) {
	QWidget* cur = views->visibleWidget();
	if ( cur ) {
	    if ( cur == dayView )
		viewWeek();
	    else if ( cur == weekView )
		viewMonth();
	    else if ( cur == monthView )
		viewDay();
	}
    }
}

void DateBook::timerEvent( QTimerEvent *e )
{
    static int stop = 0;
    if ( stop < 10 ) {
	QSound::play(Resource::findSound("alarm"));
	stop++;
    } else {
	stop = 0;
	killTimer( e->timerId() );
    }
}

void DateBook::changeClock( bool newClock )
{
    ampm = newClock;
    // repaint the affected objects...
    if (dayView) dayView->redraw();
    if (weekView) weekView->redraw();
}

void DateBook::slotToday()
{
    // we need to view today
    QDate dt = QDate::currentDate();
    showDay( dt.year(), dt.month(), dt.day() );
}

static int make_uid()
{
    // if this was multi-threaded, it would need to be
    // threadsafe
    static int last = 0;

    time_t curr = time(NULL);
    if ( last == curr ) {
	// inc it
	curr++;
    }
    last = curr;
    //retunr the negative of it...
    return -(last);
}
