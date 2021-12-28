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
#include "datebookweekheaderimpl.h"
#include <qlabel.h>
#include <qspinbox.h>
#include <qdatetime.h>

/*
 *  Constructs a DateBookWeekHeader which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 */
DateBookWeekHeader::DateBookWeekHeader( QWidget* parent,  const char* name, WFlags fl )
    : DateBookWeekHeaderBase( parent, name, fl )
{
    setBackgroundMode( PaletteButton );
    labelDate->setBackgroundMode( PaletteButton );
}

/*
 *  Destroys the object and frees any allocated resources
 */
DateBookWeekHeader::~DateBookWeekHeader()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 * public slot
 */
void DateBookWeekHeader::yearChanged( int y )
{
    setDate( y, week );
}
/*
 * public slot
 */
void DateBookWeekHeader::nextWeek()
{
    if ( week < 52 )
	week++;
    setDate( year, week );
}
/*
 * public slot
 */
void DateBookWeekHeader::prevWeek()
{
    if ( week > 1 )
	week--;
    setDate( year, week );
}
/*
 * public slot
 */
void DateBookWeekHeader::weekChanged( int w )
{
    setDate( year, w );
}

void DateBookWeekHeader::setDate( int y, int w )
{
    year = y;
    week = w;
    spinYear->setValue( y );
    spinWeek->setValue( w );

    QDate d;
    d.setYMD( year, 1, 1 );
    d = d.addDays( ( week - 1 ) * 7 - d.dayOfWeek() + 1 );

    QString s = QString::number( d.day() ) + ". " + d.monthName( d.month() ) + "-";
    d = d.addDays( 6 );
    s += QString::number( d.day() ) + ". " + d.monthName( d.month() );
    labelDate->setText( s );

    emit dateChanged( y, w );
}
