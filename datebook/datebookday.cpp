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
** $Id: datebookday.cpp,v 1.24 2001/09/05 17:13:37 twschulz Exp $
**
**********************************************************************/
#include "datebookday.h"
#include "datebookdayheaderimpl.h"
#include <datebookdb.h>
#include <resource.h>
#include <organizer.h>
#include <qpeapplication.h>
#include <timestring.h>
#include <qpedebug.h>

#include <qheader.h>
#include <qdatetime.h>
#include <qpainter.h>
#include <qsimplerichtext.h>
#include <qpopupmenu.h>
#include <qtextcodec.h>
#include <qpalette.h>

DateBookDayView::DateBookDayView( bool whichClock, QWidget *parent,
                                  const char *name )
    : QTable( 24, 1, parent, name ),
      ampm( whichClock )
{
    enableClipper(TRUE);
    setTopMargin( 0 );
    horizontalHeader()->hide();
    //setLeftMargin(34);
    setLeftMargin(38);
    setColumnStretchable( 0, TRUE );
    setHScrollBarMode( QScrollView::AlwaysOff );
    verticalHeader()->setPalette(white);
    verticalHeader()->setResizeEnabled(FALSE);

    // get rid of being able to edit things...
    QTableItem *tmp;
    int row;
    for ( row = 0; row < numRows(); row++ ) {
	tmp = new QTableItem( this, QTableItem::Never, QString::null);
	setItem( row, 0, tmp );
    }

    initHeader();
    QObject::connect( qApp, SIGNAL(clockChanged(bool)),
                      this, SLOT(slotChangeClock(bool)) );
}

void DateBookDayView::initHeader()
{
    QString strTmp;
    for ( int i = 0; i < 24; ++i ) {
	if ( ampm ) {
	    if ( i == 0 )
		strTmp = QString::number(12) + ":00";
	    else if ( i == 12 )
		strTmp = QString::number(12) + tr(":00p");
	    else if ( i > 12 )
		strTmp = QString::number( i - 12 ) + tr(":00p");
	    else
		strTmp = QString::number(i) + ":00";
	} else {
	    if ( i < 10 )
		strTmp = "0" + QString::number(i) + ":00";
	    else
		strTmp = QString::number(i) + ":00";
	}
	strTmp = strTmp.rightJustify( 6, ' ' );
	verticalHeader()->setLabel( i, strTmp );
        setRowStretchable( i, FALSE );
    }
}

void DateBookDayView::slotChangeClock( bool newClock )
{
    ampm = newClock;
    initHeader();
}

bool DateBookDayView::whichClock() const
{
    return ampm;
}

void DateBookDayView::resizeEvent( QResizeEvent *e )
{
    QTable::resizeEvent( e );
    columnWidthChanged( 0 );
    emit sigColWidthChanged();
}

DateBookDay::DateBookDay( bool ampm, DateBookDB *newDb, QWidget *parent,
                          const char *name )
    : QVBox( parent, name ),
      currDate( QDate::currentDate() ),
      db( newDb ),
      startTime( 0 )
{
    widgetList.setAutoDelete( true );
    header = new DateBookDayHeader( this, "day header" );
    header->setDate( currDate.year(), currDate.month(), currDate.day() );
    view = new DateBookDayView( ampm, this, "day view" );
    connect( header, SIGNAL( dateChanged( int, int, int ) ),
             this, SLOT( dateChanged( int, int, int ) ) );
    connect( view, SIGNAL( sigColWidthChanged() ),
             this, SLOT( slotColWidthChanged() ) );
}

void DateBookDay::selectedDates( QDateTime &start, QDateTime &end )
{
    start.setDate( currDate );
    end.setDate( currDate );

    int sh=99,eh=-1;

    int n = dayView()->numSelections();

    if ( n ) {
        for (int i=0; i<n; i++) {
            QTableSelection sel = dayView()->selection( i );
            sh = QMIN(sh,sel.topRow());
            eh = QMAX(sh,sel.bottomRow()+1);
        }
    } else {
        sh=1;
        eh=2;
    }

    start.setTime( QTime( sh, 0, 0 ) );
    end.setTime( QTime( eh, 0, 0 ) );
}

void DateBookDay::setDate( int y, int m, int d )
{
    header->setDate( y, m, d );
}

void DateBookDay::dateChanged( int y, int m, int d )
{
    currDate.setYMD( y, m, d );
    relayoutPage();
}

void DateBookDay::redraw()
{
    relayoutPage();
}

void DateBookDay::getEvents()
{
    QListIterator<DateBookDayWidget> itW( widgetList );
    for ( ; *itW; ++itW ) {
        (*itW)->hide();
    }
    widgetList.clear();
    QValueList<EffectiveEvent> eventList = db->getEffectiveEvents( currDate,
								   currDate );
    QValueListIterator<EffectiveEvent> it;
    for ( it = eventList.begin(); it != eventList.end(); ++it ) {
        DateBookDayWidget* w = new DateBookDayWidget( *it, this );
        connect( w, SIGNAL( deleteMe( const Event & ) ),
                 this, SIGNAL( removeEvent( const Event & ) ) );
        connect( w, SIGNAL( editMe( const Event & ) ),
                 this, SIGNAL( editEvent( const Event & ) ) );
        widgetList.append( w );
    }
}

void DateBookDay::relayoutPage( bool fromResize )
{
    if ( !fromResize )
        getEvents();    // no need we already have them!

    int wCount = widgetList.count();

    for ( int i = 0; i < wCount; ) {
        DateBookDayWidget *w = widgetList.at(i);
        QTime et = w->event().end();
        int n = 1;
        for ( int j = i + 1; j < wCount; j++ ) {
            QTime jst = widgetList.at(j)->event().start();
            if ( jst >= et )
                break;
            QTime jet = widgetList.at(j)->event().end();
            if ( jet > et )
                et = jet;
            n++;
        }
        int wid = ( view->columnWidth(0)-1 ) / n;
        for (int x = 0; x < n; x++) {
            w = widgetList.at(i);
            w->setX( x * wid );
            w->setWidth( wid );
            i++;
        }
    }
    view->scrollBy( 0, -height() );
    view->scrollBy( 0, startTime * view->rowHeight(0) );

}

DateBookDayWidget::DateBookDayWidget( const EffectiveEvent &e,
				      DateBookDay *db )
    : QWidget( db->dayView() ), ev( e ), dateBook( db ), xpos( 0 ), wid( 10 )
{
    place();
    bool whichClock = db->dayView()->whichClock();

    // why would someone use "<"?  Oh well, fix it up...
    // I wonder what other things may be messed up...
    QString strDesc = ev.description();
    int where = strDesc.find( "<" );
    while ( where != -1 ) {
	strDesc.remove( where, 1 );
	strDesc.insert( where, "&#60;" );
	where = strDesc.find( "<", where );
    }
    QString strCat = ev.category();
    where = strCat.find( "<" );
    while ( where != -1 ) {
	strCat.remove( where, 1 );
	strCat.insert( where, "&#60;" );
	where = strCat.find( "<", where );
    }
    
    QString strNote = ev.notes();
    where = strNote.find( "<" );
    while ( where != -1 ) {
	strNote.remove( where, 1 );
	strNote.insert( where, "&#60;" );
	where = strNote.find( "<", where );
    }
    
    text = "<b>" + strDesc + "</b><br>" + "<i>"
        + strCat + "</i>"
	+ "<br><b>" + tr("Start") + "</b>: ";


    if ( e.event().start().date() != ev.date() ) {
	// multi-day event.  Show start date
	text += e.event().start().date().toString();
    } else {
	// Show start time.
	text += timeString( ev.start(), whichClock );
    }

    text += "<br><b>" + tr("End") + "</b>: ";
    if ( e.event().end().date() != ev.date() ) {
	// multi-day event.  Show end date
	text += e.event().end().date().toString();
    } else {
	// Show end time.
	text += timeString( ev.end(), whichClock );
    }
    text += "<br><br>" + strNote;
    setBackgroundMode( PaletteBase );
}

DateBookDayWidget::~DateBookDayWidget()
{
}

void DateBookDayWidget::paintEvent( QPaintEvent *e )
{
    QPainter p( this );
    p.setPen( QColor(100, 100, 100) );
    p.setBrush( QColor( 255, 240, 230 ) ); // based on priority?
    p.drawRect(rect());

    int y = 0;
    int d = 0;

    if ( ev.event().hasAlarm() ) {
        p.drawPixmap( width() - 16, 0, Resource::loadPixmap( "bell" ) );
        y = 20;
        d = 20;
    }

    if ( ev.event().doRepeat() ) {
        p.drawPixmap( width() - 16, y, Resource::loadPixmap( "repeat" ) );
        d = 20;
    }

    QSimpleRichText rt( text, font() );
    rt.setWidth( wid - d - 6 );
    rt.draw( &p, 3, 0, e->region(), colorGroup() );
}

void DateBookDayWidget::mousePressEvent( QMouseEvent *e )
{
    QPopupMenu m;
    m.insertItem( tr( "Edit..." ), 1 );
    m.insertItem( tr( "Delete" ), 2 );
    int r = m.exec( e->globalPos() );
    if ( r == 1 ) {
        emit editMe( ev.event() );
    } else if ( r == 2 ) {
        emit deleteMe( ev.event() );
    }
}

void DateBookDayWidget::place()
{
    QTime s = ev.start();
    QTime e = ev.end();
    int y = s.hour()*60+s.minute();
    int h = e.hour()*60+e.minute()-y;
    int rh = dateBook->dayView()->rowHeight(0);
    y = y*rh/60;
    h = h*rh/60;
    if ( h < 3 )
        h = 3;
    setFixedSize( wid+1, h+1 );
    dateBook->dayView()->moveChild( this, xpos, y-1 );
    show();
}

void DateBookDayWidget::setX( int x )
{
    xpos = x;
    place();
}

void DateBookDayWidget::setWidth( int w )
{
    wid = w;
    place();
}

QDate DateBookDay::date() const
{
    return currDate;
}

void DateBookDay::setStartViewTime( int startHere )
{
    startTime = startHere;
}

int DateBookDay::startViewTime() const
{
    return startTime;
}
