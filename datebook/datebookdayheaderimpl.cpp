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
** $Id: datebookdayheaderimpl.cpp,v 1.6 2001/08/31 01:48:54 martinj Exp $
**
**********************************************************************/
#include "datebookdayheaderimpl.h"

#include <datebookmonth.h>

#include <qtimer.h>
#include <qpopupmenu.h>
#include <qtoolbutton.h>
#include <qstringlist.h>

/*
 *  Constructs a DateBookDayHeader which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
DateBookDayHeader::DateBookDayHeader( QWidget* parent,  const char* name )
    : DateBookDayHeaderBase( parent, name )
{
    connect(date,SIGNAL(pressed()),this,SLOT(pickDate()));

    sunday->setText( DateBookDayHeaderBase::tr("Sunday").left(1) );
    monday->setText( DateBookDayHeaderBase::tr("Monday").left(1) );
    tuesday->setText( DateBookDayHeaderBase::tr("Tuesday").left(1) );
    wednesday->setText( DateBookDayHeaderBase::tr("Wednesday").left(1) );
    thursday->setText( DateBookDayHeaderBase::tr("Thursday").left(1) );
    friday->setText( DateBookDayHeaderBase::tr("Friday").left(1) );
    saturday->setText( DateBookDayHeaderBase::tr("Saturday").left(1) );

    setBackgroundMode( PaletteButton );
}

/*
 *  Destroys the object and frees any allocated resources
 */
DateBookDayHeader::~DateBookDayHeader()
{
    // no need to delete child widgets, Qt does it all for us
}

void DateBookDayHeader::pickDate()
{
    static QPopupMenu *m1 = 0;
    static DateBookMonth *picker = 0;
    if ( !m1 ) {
	m1 = new QPopupMenu( this );
	picker = new DateBookMonth( m1, 0, TRUE );
	m1->insertItem( picker );
	connect( picker, SIGNAL( dateClicked( int, int, int ) ),
		 this, SLOT( setDate( int, int, int ) ) );
    }
    picker->setDate( currDate.year(), currDate.month(), currDate.day() );
    m1->popup(mapToGlobal(date->pos()+QPoint(0,date->height())));
}

/*
 * public slot
 */
void DateBookDayHeader::goBack()
{
    currDate = currDate.addDays( -1 );
    setDate( currDate.year(), currDate.month(), currDate.day() );
}
/*
 * public slot
 */
void DateBookDayHeader::goForward()
{
    currDate = currDate.addDays( 1 );
    setDate( currDate.year(), currDate.month(), currDate.day() );
}


static QString fixDateString( const QString &s )
{
    QStringList l = QStringList::split( ' ', s );
    return l[1] + " " + l[2] + " " + l[3].right( 2 );
}

/*
 * public slot
 */
void DateBookDayHeader::setDate( int y, int m, int d )
{
    currDate.setYMD( y, m, d );
    date->setText( fixDateString( currDate.toString() ) );
    monday->setOn( currDate.dayOfWeek() == 1 );
    tuesday->setOn( currDate.dayOfWeek() == 2 );
    wednesday->setOn( currDate.dayOfWeek() == 3 );
    thursday->setOn( currDate.dayOfWeek() == 4 );
    friday->setOn( currDate.dayOfWeek() == 5 );
    saturday->setOn( currDate.dayOfWeek() == 6 );
    sunday->setOn( currDate.dayOfWeek() == 7 );
    emit dateChanged( y, m, d );
}
/*
 * public slot
 */
void DateBookDayHeader::setFriday()
{
    if ( currDate.dayOfWeek() < 5 ) {
	while ( currDate.dayOfWeek() < 5 )
	    goForward();
    } else if ( currDate.dayOfWeek() > 5 ) {
	while ( currDate.dayOfWeek() > 5 )
	    goBack();
    } else {
	setDate( currDate.year(), currDate.month(), currDate.day() );
    }
}
/*
 * public slot
 */
void DateBookDayHeader::setMonday()
{
    if ( currDate.dayOfWeek() < 1 ) {
	while ( currDate.dayOfWeek() < 1 )
	    goForward();
    } else if ( currDate.dayOfWeek() > 1 ) {
	while ( currDate.dayOfWeek() > 1 )
	    goBack();
    } else {
	setDate( currDate.year(), currDate.month(), currDate.day() );
    }
}
/*
 * public slot
 */
void DateBookDayHeader::setSaturday()
{
    if ( currDate.dayOfWeek() < 6 ) {
	while ( currDate.dayOfWeek() < 6 )
	    goForward();
    } else if ( currDate.dayOfWeek() > 6 ) {
	while ( currDate.dayOfWeek() > 6 )
	    goBack();
    } else {
	setDate( currDate.year(), currDate.month(), currDate.day() );
    }
}
/*
 * public slot
 */
void DateBookDayHeader::setSunday()
{
    if ( currDate.dayOfWeek() < 7 ) {
	while ( currDate.dayOfWeek() < 7 )
	    goForward();
    } else if ( currDate.dayOfWeek() > 7 ) {
	while ( currDate.dayOfWeek() > 7 )
	    goBack();
    } else {
	setDate( currDate.year(), currDate.month(), currDate.day() );
    }
}
/*
 * public slot
 */
void DateBookDayHeader::setThursday()
{
    if ( currDate.dayOfWeek() < 4 ) {
	while ( currDate.dayOfWeek() < 4 )
	    goForward();
    } else if ( currDate.dayOfWeek() > 4 ) {
	while ( currDate.dayOfWeek() > 4 )
	    goBack();
    } else {
	setDate( currDate.year(), currDate.month(), currDate.day() );
    }
}
/*
 * public slot
 */
void DateBookDayHeader::setTuesday()
{
    if ( currDate.dayOfWeek() < 2 ) {
	while ( currDate.dayOfWeek() < 2 )
	    goForward();
    } else if ( currDate.dayOfWeek() > 2 ) {
	while ( currDate.dayOfWeek() > 2 )
	    goBack();
    } else {
	setDate( currDate.year(), currDate.month(), currDate.day() );
    }
}
/*
 * public slot
 */
void DateBookDayHeader::setWednesday()
{
    if ( currDate.dayOfWeek() < 3 ) {
	while ( currDate.dayOfWeek() < 3 )
	    goForward();
    } else if ( currDate.dayOfWeek() > 3 ) {
	while ( currDate.dayOfWeek() > 3 )
	    goBack();
    } else {
	setDate( currDate.year(), currDate.month(), currDate.day() );
    }
}

