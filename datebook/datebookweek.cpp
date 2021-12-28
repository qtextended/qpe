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
** $Id: datebookweek.cpp,v 1.29 2001/09/05 17:13:37 twschulz Exp $
**
**********************************************************************/
#include "datebookweek.h"
#include "datebookweekheaderimpl.h"

#include <calendar.h>
#include <datebookdb.h>
#include <organizer.h>
#include <qpeapplication.h>
#include <timestring.h>

#include <qdatetime.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpopupmenu.h>
#include <qtimer.h>
#include <qstyle.h>

//------------------------------------------------------------------------------


DateBookWeekItem::DateBookWeekItem( const EffectiveEvent e )
    : ev( e )
{
    // with the current implementation change the color for all day events
    if ( ev.event().type() == Event::AllDay ) {
        c = Qt::green;
    } else {
        c = ev.event().hasAlarm() ? Qt::red : Qt::blue;
    }
}

void DateBookWeekItem::setGeometry( int x, int y, int w, int h )
{
    r.setRect( x, y, w, h );
}


//------------------------------------------------------------------------------

DateBookWeekView::DateBookWeekView( bool ap, QWidget *parent, const char *name )
    : QScrollView( parent, name ), ampm( ap ), showingEvent( false )
{
    items.setAutoDelete( true );

    viewport()->setBackgroundMode( PaletteBase );

    header = new QHeader( this );
    header->setMovingEnabled( false );
    header->setResizeEnabled( false );
    header->addLabel( "" );
    header->addLabel( tr("M") );
    header->addLabel( tr("T") );
    header->addLabel( tr("W") );
    header->addLabel( tr("T") );
    header->addLabel( tr("F") );
    header->addLabel( tr("S") );
    header->addLabel( tr("S") );
    header->setClickEnabled( false, 0 );
    connect( header, SIGNAL(clicked(int)), this, SIGNAL(showDay(int)) );

    QFontMetrics fm( font() );
    rowHeight = fm.height()+2;

    resizeContents( width(), 24*rowHeight );
}

void DateBookWeekView::showEvents( QValueList<EffectiveEvent> &ev )
{
    items.clear();
    QValueListIterator<EffectiveEvent> it;
    for ( it = ev.begin(); it != ev.end(); ++it ) {
	DateBookWeekItem *i = new DateBookWeekItem( *it );
	positionItem( i );
	items.append( i );
    }
    viewport()->update();
}

void DateBookWeekView::moveToHour( int h )
{
    int offset = h*rowHeight;
    setContentsPos( 0, offset );
}

void DateBookWeekView::slotChangeClock( bool c )
{
    ampm = c;
    viewport()->repaint();
}

static inline int db_round30min( int m )
{
    if ( m < 15 )
	m = 0;
    else if ( m < 45 )
	m = 1;
    else
	m = 2;

    return m;
}

void DateBookWeekView::positionItem( DateBookWeekItem *i )
{
    const int Width = 8;
    const EffectiveEvent ev = i->event();

    // 30 minute intervals
    int y = ev.start().hour() * 2;
    y += db_round30min( ev.start().minute() );
    y = y * rowHeight / 2;

    int h;
    if ( ev.event().type() == Event::AllDay ) {
	h = 48;
	y = 0;
    } else {
	h = ( ev.end().hour() - ev.start().hour() ) * 2;
	h += db_round30min( ev.end().minute() - ev.start().minute() );
	if ( h < 1 ) h = 1;
    }
    h = h * rowHeight / 2;

    int dow = ev.date().dayOfWeek();
    int x = header->sectionPos( dow ) - 1;
    int xlim = header->sectionPos( dow ) + header->sectionSize( dow );
    DateBookWeekItem *isect = 0;
    do {
	i->setGeometry( x, y, Width, h );
	isect = intersects( i );
	x += Width - 1;
    } while ( isect && x < xlim );
}

DateBookWeekItem *DateBookWeekView::intersects( const DateBookWeekItem *item )
{
    QRect geom = item->geometry();

    // We allow the edges to overlap
    geom.moveBy( 1, 1 );
    geom.setSize( geom.size()-QSize(2,2) );

    QListIterator<DateBookWeekItem> it(items);
    for ( ; it.current(); ++it ) {
	DateBookWeekItem *i = it.current();
	if ( i != item ) {
	    if ( i->geometry().intersects( geom ) ) {
		return i;
	    }
	}
    }

    return 0;
}

void DateBookWeekView::contentsMousePressEvent( QMouseEvent *e )
{
    QListIterator<DateBookWeekItem> it(items);
    for ( ; it.current(); ++it ) {
	DateBookWeekItem *i = it.current();
	if ( i->geometry().contains( e->pos() ) ) {
	    showingEvent = true;
	    emit signalShowEvent( i->event() );
	    break;
	}
    }
}

void DateBookWeekView::contentsMouseReleaseEvent( QMouseEvent * )
{
    if ( showingEvent ) {
	showingEvent = false;
	emit signalHideEvent();
    }
}

void DateBookWeekView::drawContents( QPainter *p, int cx, int cy, int cw, int ch )
{
    QRect ur( cx, cy, cw, ch );
    p->setPen( lightGray );
    for ( int i = 1; i <= 7; i++ )
	p->drawLine( header->sectionPos(i)-2, cy, header->sectionPos(i)-2, cy+ch );

    p->setPen( black );
    for ( int t = 0; t < 24; t++ ) {
	int y = t*rowHeight;
	if ( QRect( 1, y, 20, rowHeight ).intersects( ur ) ) {
	    QString s;
	    if ( ampm ) {
		if ( t == 0 )
		    s = QString::number( 12 );
		else if ( t > 13 )
		    s = QString::number( t - 12 );
		else
		    s = QString::number( t );
	    }
	    else {
		s = QString::number( t );
		if ( s.length() == 1 )
		    s.prepend( "0" );
	    }
	    p->drawText( 1, y+p->fontMetrics().ascent()+1, s );
	}
    }

    QListIterator<DateBookWeekItem> it(items);
    for ( ; it.current(); ++it ) {
	DateBookWeekItem *i = it.current();
	if ( i->geometry().intersects( ur ) ) {
	    p->setBrush( i->color() );
	    p->drawRect( i->geometry() );
	}
    }
}

void DateBookWeekView::resizeEvent( QResizeEvent *e )
{
    const int hourWidth = 20;
    QScrollView::resizeEvent( e );
    int avail = width()-qApp->style().scrollBarExtent().width()-1;
    header->setGeometry( 0, 0, avail, header->sizeHint().height() );
    setMargins( 0, header->height(), 0, 0 );
    header->resizeSection( 0, hourWidth );
    int sw = (avail - hourWidth) / 7;
    for ( int i = 1; i < 7; i++ )
	header->resizeSection( i, sw );
    header->resizeSection( 7, avail - hourWidth - sw*6 );
}

//------------------------------------------------------------------------------

DateBookWeek::DateBookWeek( bool ap, DateBookDB *newDB, QWidget *parent, const char *name )
    : QWidget( parent, name ),
      db( newDB ),
      startTime( 0 ),
      ampm( ap )
{
    QVBoxLayout *vb = new QVBoxLayout( this );
    header = new DateBookWeekHeader( this );
    view = new DateBookWeekView( ampm, this );
    vb->addWidget( header );
    vb->addWidget( view );

    lblDesc = new QLabel( this, "event label" );
    lblDesc->setFrameStyle( QFrame::Plain | QFrame::Box );
    lblDesc->setBackgroundColor( yellow );
    lblDesc->hide();

    tHide = new QTimer( this );

    connect( view, SIGNAL( showDay( int ) ),
             this, SLOT( showDay( int ) ) );
    connect( view, SIGNAL(signalShowEvent(const EffectiveEvent&)),
	     this, SLOT(slotShowEvent(const EffectiveEvent&)) );
    connect( view, SIGNAL(signalHideEvent()),
	     this, SLOT(slotHideEvent()) );
    connect( header, SIGNAL( dateChanged( int, int ) ),
             this, SLOT( dateChanged( int, int ) ) );
    connect( tHide, SIGNAL( timeout() ),
             lblDesc, SLOT( hide() ) );
    setDate( QDate::currentDate().year(), (QDate::currentDate().dayOfYear() / 7 ) + 1 );
}

void DateBookWeek::showDay( int day )
{
    QDate d;
    d.setYMD( year, 1, 1 );
    d = d.addDays( ( _week - 1 ) * 7 - d.dayOfWeek() + 1 );
    d = d.addDays( day - 1 );
    emit showDate( d.year(), d.month(), d.day() );
}

void DateBookWeek::setDate( int y, int w )
{
    header->setDate( y, w );
}

void DateBookWeek::dateChanged( int y, int w )
{
    year = y;
    _week = w;
    getEvents();
}

QDate DateBookWeek::date() const
{
    QDate d;
    d.setYMD( year, 1, 1 );
    d = d.addDays( ( _week - 1 ) * 7 - d.dayOfWeek() + 1 );
    return d;
}

void DateBookWeek::getEvents()
{
    QDate endWeek = date().addDays( 6 );
    QValueList<EffectiveEvent> eventList = db->getEffectiveEvents(date(),
								  endWeek);
    view->showEvents( eventList );
    view->moveToHour( startTime );
}

void DateBookWeek::slotShowEvent( const EffectiveEvent &ev )
{
    if ( tHide->isActive() )
        tHide->stop();

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

    QString str = "<b>" + strDesc + "</b><br>" + "<i>"
            + strCat + "</i>"
	    + "<br>" + ev.date().toString()
	    + "<br><b>" + QObject::tr("Start") + "</b>: ";

    if ( ev.event().start().date() != ev.date() ) {
	// multi-day event.  Show start date
	str += ev.event().start().date().toString();
    } else {
	// Show start time.
	str += timeString(ev.start(), ampm );
    }

    str += "<br><b>" + QObject::tr("End") + "</b>: ";
    if ( ev.event().end().date() != ev.date() ) {
	// multi-day event.  Show end date
	str += ev.event().end().date().toString();
    } else {
	// Show end time.
	str += timeString( ev.end(), ampm );
    }
    str += "<br><br>" + strNote;

    lblDesc->setText( str );
    lblDesc->setMinimumSize( lblDesc->sizeHint() );
    // move the label so it is "centerd" horizontally...
    lblDesc->move( width() / 2 - lblDesc->width() / 2, 0 );
    lblDesc->show();
}

void DateBookWeek::slotHideEvent()
{
    tHide->start( 2000, true );
}

void DateBookWeek::setStartViewTime( int startHere )
{
    startTime = startHere;
    view->moveToHour( startTime );
}

int DateBookWeek::startViewTime() const
{
    return startTime;
}

void DateBookWeek::redraw()
{
    getEvents();
}

