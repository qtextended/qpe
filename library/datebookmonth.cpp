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
#include "datebookmonth.h"
#include "datebookdb.h"
#include "organizer.h"
#include "resource.h"

#include <qtoolbutton.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qdatetime.h>
#include <qpainter.h>

DateBookMonthHeader::DateBookMonthHeader( QWidget *parent, const char *name )
    : QHBox( parent, name )
{
    setBackgroundMode( PaletteButton );

    begin = new QToolButton( this );
    begin->setPixmap( Resource::loadPixmap( "start" ) );
    begin->setAutoRaise( TRUE );
    begin->setFixedSize( begin->sizeHint() );

    back = new QToolButton( this );
    back->setPixmap( Resource::loadPixmap( "back" ) );
    back->setAutoRaise( TRUE );
    back->setFixedSize( back->sizeHint() );

    month = new QComboBox( FALSE, this );
    for ( int i = 0; i < 12; ++i )
	month->insertItem( Calendar::nameOfMonth( i + 1 ) );

    year = new QSpinBox( 1900, 3000, 1, this );

    next = new QToolButton( this );
    next->setPixmap( Resource::loadPixmap( "forward" ) );
    next->setAutoRaise( TRUE );
    next->setFixedSize( next->sizeHint() );

    end = new QToolButton( this );
    end->setPixmap( Resource::loadPixmap( "finish" ) );
    end->setAutoRaise( TRUE );
    end->setFixedSize( end->sizeHint() );

    connect( month, SIGNAL( activated( int ) ),
	     this, SLOT( updateDate() ) );
    connect( year, SIGNAL( valueChanged( int ) ),
	     this, SLOT( updateDate() ) );
    connect( begin, SIGNAL( clicked() ),
	     this, SLOT( firstMonth() ) );
    connect( end, SIGNAL( clicked() ),
	     this, SLOT( lastMonth() ) );
    connect( back, SIGNAL( clicked() ),
	     this, SLOT( monthBack() ) );
    connect( next, SIGNAL( clicked() ),
	     this, SLOT( monthForward() ) );
    back->setAutoRepeat( TRUE );
    next->setAutoRepeat( TRUE );
}


DateBookMonthHeader::~DateBookMonthHeader()
{

}

void DateBookMonthHeader::updateDate()
{
    emit dateChanged( year->value(), month->currentItem() + 1 );
}

void DateBookMonthHeader::firstMonth()
{
    emit dateChanged( year->value(), 1 );
    month->setCurrentItem( 0 );
}

void DateBookMonthHeader::lastMonth()
{
    emit dateChanged( year->value(), 12 );
    month->setCurrentItem( 11 );
}

void DateBookMonthHeader::monthBack()
{
    if ( month->currentItem() > 0 ) {
	emit dateChanged( year->value(), month->currentItem() );
	month->setCurrentItem( month->currentItem() - 1 );
    } else {
	emit dateChanged( year->value() - 1, 12 );
	year->setValue( year->value() - 1 );
	month->setCurrentItem( 11 );
    }
}

void DateBookMonthHeader::monthForward()
{
    if ( month->currentItem() < 11 ) {
	emit dateChanged( year->value(), month->currentItem() + 2 );
	month->setCurrentItem( month->currentItem() + 1 );
    } else {
	emit dateChanged( year->value() + 1, 1 );
	year->setValue( year->value() + 1 );
	month->setCurrentItem( 0 );
    }
}

void DateBookMonthHeader::setDate( int y, int m )
{
    year->setValue( y );
    month->setCurrentItem( m - 1 );
}

//---------------------------------------------------------------------------

class DateBookMonthTablePrivate
{
public:
    DateBookMonthTablePrivate() {};
    ~DateBookMonthTablePrivate() { mMonthEvents.clear(); };

    QValueList<EffectiveEvent> mMonthEvents;
};

DateBookMonthTable::DateBookMonthTable( QWidget *parent, const char *name,
                                        DateBookDB *newDb  )
    : QTable( 6, 7, parent, name ),
      db( newDb )
{
    d = new DateBookMonthTablePrivate();
    horizontalHeader()->setResizeEnabled( FALSE );
    for ( int i = 0; i < 7; ++i ) {
	horizontalHeader()->resizeSection( i, 30 );
	setColumnStretchable( i, TRUE );
	horizontalHeader()->setLabel( i, Calendar::nameOfDay( i + 1 ) );
    }

    verticalHeader()->hide();
    setLeftMargin( 0 );
    for ( int i = 0; i < 6; ++i )
	    setRowStretchable( i, TRUE );

    setSelectionMode( NoSelection );

    connect( this, SIGNAL( clicked( int, int, int, const QPoint & ) ),
	     this, SLOT( dayClicked( int, int ) ) );
    connect( this, SIGNAL( currentChanged( int, int ) ),
             this, SLOT( dragDay( int, int ) ) );
    setVScrollBarMode( AlwaysOff );
    setHScrollBarMode( AlwaysOff );
}

DateBookMonthTable::~DateBookMonthTable()
{
    monthsEvents.clear();
    delete d;
}

void DateBookMonthTable::setupTable()
{
    QValueList<Calendar::Day> days = Calendar::daysOfMonth( year, month, FALSE );
    QValueList<Calendar::Day>::Iterator it = days.begin();
    int row = 0, col = 0;
    int crow = 0;
    int ccol = 0;
    for ( ; it != days.end(); ++it ) {
	DayItemMonth *i = (DayItemMonth *)item( row, col );
	if ( !i ) {
	    i = new DayItemMonth( this, QTableItem::Never, "" );
	    setItem( row, col, i );
	}
	Calendar::Day calDay = *it;
	i->clearEffEvents();
	i->setDay( calDay.date );
	i->setType( calDay.type );
	if ( i->day() == day && calDay.type == Calendar::Day::ThisMonth ) {
	    crow = row;
	    ccol = col;
	}

	updateCell( row, col );

	if ( col == 6 ) {
	    ++row;
	    col = 0;
	} else {
	    ++col;
	}
    }
    setCurrentCell( crow, ccol );
    getEvents();
}

void DateBookMonthTable::findDay( int day, int &row, int &col )
{
    // a brute force way of finding the row and column the day is on the
    // row and column are returned back to the user
    QDate dtTmp( year, month, day );
    QDate dtBegin( year, month, 1 );
    int stop = dtBegin.daysTo( dtTmp );
    int tracker = dtBegin.dayOfWeek();
    col = dtTmp.dayOfWeek() - 1;
    row = 0;
    if ( tracker == 1 )
	row++;
    for ( int i = 0; i < stop; i++ ) {
	if ( ++tracker >= 7 ) {
	    if ( dtTmp.dayOfWeek() != tracker ) {
		tracker = 0;
		row++;
	    }
	}
    }
}

void DateBookMonthTable::dayClicked( int row, int col )
{
    changeDaySelection( row, col );
    emit dateClicked( selYear, selMonth,  selDay );
}

void DateBookMonthTable::dragDay( int row, int col )
{
    changeDaySelection( row, col );
}

void DateBookMonthTable::changeDaySelection( int row, int col )
{
    DayItemMonth *i = (DayItemMonth*)item( row, col );
    if ( !i )
	return;
    selMonth = i->type() == Calendar::Day::ThisMonth ? month : i->type() == Calendar::Day::PrevMonth ? month - 1 : month + 1;

    selYear = year;
    if ( selMonth <= 0 ) {
	selMonth = 12;
	selYear--;
    } else if ( selMonth > 12 ) {
	selMonth = 1;
	selYear++;
    }
    selDay = i->day();
}


void DateBookMonthTable::viewportMouseReleaseEvent( QMouseEvent * )
{
    dayClicked( currentRow(), currentColumn() );
}

void DateBookMonthTable::getEvents()
{
    if ( !db )
	return;

    QDate dtStart( year, month, 1 );
    d->mMonthEvents = db->getEffectiveEvents( dtStart,
					      QDate( year, month,
						     dtStart.daysInMonth() ) );
    QValueListIterator<EffectiveEvent> it = d->mMonthEvents.begin();
    // now that the events are sorted, basically go through the list, make
    // a small list for every day and set it for each item...
    // clear all the items...
    while ( it != d->mMonthEvents.end() ) {
	QValueList<EffectiveEvent> dayEvent;
	EffectiveEvent e = *it;
	++it;
	dayEvent.append( e );
	while ( it != d->mMonthEvents.end()
	        && e.date() == (*it).date() ) {
	    dayEvent.append( *it );
	    ++it;
	}
	int row, col;
	findDay( e.date().day(), row, col );
	DayItemMonth* w = static_cast<DayItemMonth*>( item( row, col ) );
	w->setEvents( dayEvent );
	updateCell( row, col );
	dayEvent.clear();
    }
}


//---------------------------------------------------------------------------

DateBookMonth::DateBookMonth( QWidget *parent, const char *name, bool ac,
                              DateBookDB *data )
    : QVBox( parent, name ),
      autoClose( ac )
{
    year = QDate::currentDate().year();
    month = QDate::currentDate().month();
    header = new DateBookMonthHeader( this, "DateBookMonthHeader" );
    table = new DateBookMonthTable( this, "DateBookMonthTable", data );
    header->setDate( year, month );
    table->setDate( year, month, QDate::currentDate().day() );
    connect( header, SIGNAL( dateChanged( int, int ) ),
	     this, SLOT( setDate( int, int ) ) );
    connect( table, SIGNAL( dateClicked( int, int, int ) ),
	     this, SIGNAL( dateClicked( int, int, int ) ) );
    connect( table, SIGNAL( dateClicked( int, int, int ) ),
	     this, SLOT( forwardDateClicked( int, int, int ) ) );
    if ( ac )
	connect( table, SIGNAL( dateClicked( int, int, int ) ),
		 this, SLOT( setDate( int, int ) ) );
}

DateBookMonth::~DateBookMonth()
{

}

void DateBookMonth::setDate( int y, int m )
{
    year = y;
    month = m;
    day = 1;
    table->setDate( year, month, 1 );
    if ( autoClose && parentWidget() && sender() == table )
	parentWidget()->close();
}

void DateBookMonth::setDate( int y, int m, int d )
{
    year = y;
    month = m;
    day = d;
    header->setDate( year, month );
    table->setDate( year, month, day );
    if ( autoClose && parentWidget() && sender() == table )
	parentWidget()->close();
}

QDate  DateBookMonth::selectedDate() const
{
    if ( !table )
	return QDate::currentDate();
    int y, m, d;
    table->getDate( y, m, d );
    return QDate( y, m, d );
}

//---------------------------------------------------------------------------
class DayItemMonthPrivate
{
public:
    DayItemMonthPrivate() {};
    ~DayItemMonthPrivate() { mDayEvents.clear(); };
    QValueList<EffectiveEvent> mDayEvents;
};

DayItemMonth::DayItemMonth( QTable *table, EditType et, const QString &t )
	: QTableItem( table, et, t )
{
    d = new DayItemMonthPrivate();
}

DayItemMonth::~DayItemMonth()
{
    daysEvents.clear();
    delete d;
}

void DayItemMonth::setEvents( const QValueList<EffectiveEvent> &effEv )
{
    d->mDayEvents = effEv;
}

void DayItemMonth::clearEffEvents()
{
    d->mDayEvents.clear();
}

void DayItemMonth::paint( QPainter *p, const QColorGroup &cg,
                          const QRect &cr, bool selected )
{
    QColorGroup g( cg );
    g.setBrush( QColorGroup::Base, back );
    g.setColor( QColorGroup::Text, forg );
    p->fillRect( 0, 0, cr.width(), cr.height(), selected ? g.brush( QColorGroup::Highlight ) : g.brush( QColorGroup::Base ) );

    if ( selected )
	p->setPen( g.highlightedText() );
    else
	p->setPen( g.text() );

    p->save();
    QFont f = p->font();
    f.setPointSize( ( f.pointSize() / 3 ) * 2 );
    p->setFont( f );
    QFontMetrics fm( f );
    p->drawText( 1, 1 + fm.ascent(), QString::number( day() ) );
    p->restore();
    // Put indicators for something like this, (similar to PalmOS)
    // Before noon: item at top of the day
    // At noon: put a small item at the middle
    // After noon: put an indicator at the bottom of the day
    // an all day event: mark with a circle in the middle (a la DateBook+)
    bool beforeNoon,
         atNoon,
	 afterNoon,
	 bAllDay,
	 bRepeat;
    beforeNoon = atNoon = afterNoon = bAllDay = bRepeat = FALSE;
    QValueListIterator<EffectiveEvent> itDays = d->mDayEvents.begin();
    for ( ; itDays != d->mDayEvents.end(); ++itDays ) {
	if ( (*itDays).event().type() == Event::AllDay && !bAllDay )
	    bAllDay = TRUE;
	else if ( (*itDays).start().hour() < 12 && !beforeNoon )
	    beforeNoon = TRUE;
	else if ( (*itDays).start().hour() == 12 ) {
	    if ( !atNoon )
		atNoon = TRUE;
	    if ( (*itDays).start().minute() > 0
		 || (*itDays).end().hour() > 13 )
		afterNoon = TRUE;
	} else if ( (*itDays).start().hour() > 12 && !afterNoon )
	    afterNoon = TRUE;
	if ( (*itDays).event().doRepeat() )
	    bRepeat = TRUE;
    }
    if ( beforeNoon ) {
	p->setBrush( blue );
	p->drawRect( cr.width() - 13, 2,  10, 10 );
	if ( bRepeat )
	    p->fillRect( cr.width() - 8, 4,  3, 3, white );
    }
    if ( atNoon ) {
	p->setBrush( blue );
	p->drawRect( cr.width() - 13, 14, 10, 5 );
	if ( bRepeat )
	    p->fillRect( cr.width() - 8, 16,  3, 2, white );
    }
    if ( afterNoon ) {
	p->setBrush( blue );
	p->drawRect( cr.width() - 13, 21, 10, 10 );
	if ( bRepeat )
	    p->fillRect( cr.width() - 8, 23,  3, 3, white );
    }
    if ( bAllDay ) {
	p->setBrush( green );
	p->drawEllipse( cr.width() / 2 - 7, cr.height() / 2 - 5, 10, 10 );
    }
}



void DayItemMonth::setType( Calendar::Day::Type t )
{
    switch ( t ) {
    case Calendar::Day::PrevMonth:
    case Calendar::Day::NextMonth:
	back = QBrush( QColor( 224, 224, 224 ) );
	forg = black;
	break;
    case Calendar::Day::ThisMonth:
	back = QBrush( white );
	forg = black;
	break;
    }
    typ = t;
}
