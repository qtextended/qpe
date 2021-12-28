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
** $Id: repeatentry.cpp,v 1.21 2001/08/28 18:29:11 twschulz Exp $
**
**********************************************************************/

#include "repeatentry.h"

#include <datebookmonth.h>
#include <qbuttongroup.h>
#include <qlabel.h>
#include <qpopupmenu.h>
#include <qspinbox.h>
#include <qtoolbutton.h>

#include <ctime>

// Global Templates for use in setting up the repeat label...
const QString strDayTemplate = QObject::tr("Every");
const QString strYearTemplate = QObject::tr("%1 %2 every ");
const QString strMonthDateTemplate = QObject::tr("The %1 every ");
const QString strMonthDayTemplate = QObject::tr("The %1 %1 of every");
const QString strWeekTemplate = QObject::tr("Every ");
const QString dayLabel[] = { QObject::tr("Monday"),
                             QObject::tr("Tuesday"),
			     QObject::tr("Wednesday"),
			     QObject::tr("Thursday"),
			     QObject::tr("Friday"),
			     QObject::tr("Saturday"),
			     QObject::tr("Sunday") };

static QString fixDateString( const QString &s );
static QString numberPlacing( int x );	// return the proper word format for
                                        // x (1st, 2nd, etc)
static int week( const QDate &dt );    // what week in the month is dt?

RepeatEntry::RepeatEntry( const QDate &newStart, QWidget *parent,
                          const char *name, bool modal, WFlags fl )
    : RepeatEntryBase( parent, name, modal, fl ),
      start( newStart ),
      currInterval( NONE )
{
    init();
    fraType->setButton( currInterval );
    chkNoEnd->setChecked( TRUE );
    setupNone();
}

RepeatEntry::RepeatEntry( const Event::RepeatPattern &rp,
                          const QDate &startDate,
                          QWidget *parent, const char *name, bool modal,
			  WFlags fl )
    : RepeatEntryBase( parent, name, modal, fl ),
      start( startDate ),
      end( rp.endDate )
{
    // do some stuff with the repeat pattern
    init();
    switch ( rp.type ) {
	default:
	case Event::NoRepeat:
	    currInterval = NONE;
	    setupNone();
	    break;
	case Event::Daily:
	    currInterval = DAY;
	    setupDaily();
	    break;
	case Event::Weekly:
	    currInterval = WEEK;
	    setupWeekly();
	    int day, buttons;
	    for ( day = 0x01, buttons = 0; buttons < 7;
	          day = day << 1, buttons++ ) {
		if ( rp.days & day )
		    fraExtra->setButton( buttons );
	    }
	    slotWeekLabel( -1 );
	    break;
	case Event::MonthlyDay:
	    currInterval = MONTH;
	    setupMonthly();
	    fraExtra->setButton( 0 );
	    slotMonthLabel( 0 );
	    break;
	case Event::MonthlyDate:
	    currInterval = MONTH;
	    setupMonthly();
	    fraExtra->setButton( 1 );
	    slotMonthLabel( 1 );
	    break;
	case Event::Yearly:
	    currInterval = YEAR;
	    setupYearly();
	    break;
    }
    fraType->setButton( currInterval );
    spinFreq->setValue( rp.frequency );
    if ( !rp.hasEndDate ) {
	cmdEnd->setText( RepeatEntryBase::tr("No End Date") );
	chkNoEnd->setChecked( TRUE );
    } else
	cmdEnd->setText( fixDateString( end.toString() ) );
}

RepeatEntry::~RepeatEntry()
{
}

Event::RepeatPattern RepeatEntry::repeatPattern()
{
    QListIterator<QToolButton> it( listRTypeButtons );
    QListIterator<QToolButton> itExtra( listExtra );
    Event::RepeatPattern rpTmp;
    int i;
    for ( i = 0; *it; ++it, i++ ) {
	if ( (*it)->isOn() ) {
	    switch ( i ) {
		case NONE:
		   rpTmp.type = Event::NoRepeat;
		   break;
		case DAY:
		    rpTmp.type = Event::Daily;
		    break;
		case WEEK:
		    rpTmp.type = Event::Weekly;
		    rpTmp.days = 0;
		    int day;
		    for ( day = 1; *itExtra; ++itExtra, day = day << 1 ) {
			if ( (*itExtra)->isOn() )
			    rpTmp.days |= day;
		    }
		    break;
		case MONTH:
		    if ( cmdExtra1->isOn() )
			rpTmp.type = Event::MonthlyDay;
		    else if ( cmdExtra2->isOn() )
			rpTmp.type = Event::MonthlyDate;
			// figure out the montly day...
			rpTmp.position = week( start );
		    break;
		case YEAR:
		    rpTmp.type = Event::Yearly;
		    break;
	    }
	    break;  // no need to keep looking!
	}
    }
    rpTmp.frequency = spinFreq->value();
    rpTmp.hasEndDate = !chkNoEnd->isChecked();
    if ( rpTmp.hasEndDate ) {
	rpTmp.endDate = end;
    }
    // timestamp it...
    rpTmp.createTime = time( NULL );
    return rpTmp;
}

void RepeatEntry::slotSetRType( int rtype )
{
    // now call the right function based on the type...
    currInterval = static_cast<repeatButtons>(rtype);
    switch ( currInterval ) {
	case NONE:
	    setupNone();
	    break;
	case DAY:
	    setupDaily();
	    break;
	case WEEK:
	    setupWeekly();
	    slotWeekLabel( -1 );
	    break;
	case MONTH:
	    setupMonthly();
	    cmdExtra2->setOn( TRUE );
	    slotMonthLabel( 1 );
	    break;
	case YEAR:
	    setupYearly();
	    break;
    }
}

void RepeatEntry::setupNone()
{
    lblRepeat->setText( tr("No Repeat") );
    lblVar1->hide();
    lblVar2->hide();
    hideExtras();
    cmdEnd->hide();
    lblFreq->hide();
    lblEvery->hide();
    lblFreq->hide();
    spinFreq->hide();
    lblEnd->hide();
    lblWeekVar->hide();
}


void RepeatEntry::setupDaily()
{
    hideExtras();
    lblWeekVar->hide();
    spinFreq->setValue( 1 );
    lblFreq->setText( tr("day(s)") );
    lblVar2->show();
    showRepeatStuff();
    lblRepeat->setText( strDayTemplate );
    setupRepeatLabel( 1 );
}

void RepeatEntry::setupWeekly()
{
    // reshow the buttons...
    fraExtra->setTitle( RepeatEntryBase::tr("Repaet On") );
    fraExtra->setExclusive( FALSE );
    fraExtra->show();
    cmdExtra1->setText( RepeatEntryBase::tr("Mon") );
    cmdExtra1->setOn( FALSE );
    cmdExtra1->show();
    cmdExtra2->setText( RepeatEntryBase::tr("Tue") );
    cmdExtra2->setOn( FALSE );
    cmdExtra2->show();
    cmdExtra3->setText( RepeatEntryBase::tr("Wed") );
    cmdExtra3->setOn( FALSE );
    cmdExtra3->show();
    cmdExtra4->setText( RepeatEntryBase::tr("Thu") );
    cmdExtra4->setOn( FALSE );
    cmdExtra4->show();
    cmdExtra5->setText( RepeatEntryBase::tr("Fri") );
    cmdExtra5->setOn( FALSE );
    cmdExtra5->show();
    cmdExtra6->setText( RepeatEntryBase::tr("Sat") );
    cmdExtra6->setOn( FALSE );
    cmdExtra6->show();
    cmdExtra7->setText( RepeatEntryBase::tr("Sun") );
    cmdExtra7->setOn( FALSE );
    cmdExtra7->show();
    lblWeekVar->show();
    spinFreq->setValue( 1 );
    // might as well set the day too...
    fraExtra->setButton( start.dayOfWeek() - 1 );
    lblFreq->setText( tr("week(s)") );
    lblVar2->show();
    showRepeatStuff();
    setupRepeatLabel( 1 );
}

void RepeatEntry::setupMonthly()
{
    hideExtras();
    lblWeekVar->hide();
    fraExtra->setTitle( tr("Repeat By") );
    fraExtra->setExclusive( TRUE );
    fraExtra->show();
    cmdExtra1->setText( tr("Day") );
    cmdExtra1->show();
    cmdExtra2->setText( tr("Date") );
    cmdExtra2->show();
    spinFreq->setValue( 1 );
    lblFreq->setText( tr("month(s)") );
    lblVar2->show();
    showRepeatStuff();
    setupRepeatLabel( 1 );
}

void RepeatEntry::setupYearly()
{
    hideExtras();
    lblWeekVar->hide();
    spinFreq->setValue( 1 );
    lblFreq->setText( tr("year(s)") );
    lblFreq->show();
    lblFreq->show();
    showRepeatStuff();
    lblVar2->show();
    QString strEvery = strYearTemplate.arg( start.monthName(start.month()) ).arg( numberPlacing(start.day()) );
    lblRepeat->setText( strEvery );
    setupRepeatLabel( 1 );

}

void RepeatEntry::init()
{
    DateBookMonth *picker;
    QPopupMenu *m1 = new QPopupMenu( this );
    picker = new DateBookMonth( m1, 0, TRUE );
    m1->insertItem( picker );
    cmdEnd->setPopup( m1 );
    cmdEnd->setPopupDelay( 0 );

    QObject::connect( picker, SIGNAL(dateClicked(int, int, int)),
                      this, SLOT(endDateChanged(int, int, int)) );
    QObject::connect( picker, SIGNAL(dateClicked( const QString &)),
                      this, SLOT(endDateChanged(const QString &)) );

    listRTypeButtons.setAutoDelete( TRUE );
    listRTypeButtons.append( cmdNone );
    listRTypeButtons.append( cmdDay );
    listRTypeButtons.append( cmdWeek );
    listRTypeButtons.append( cmdMonth );
    listRTypeButtons.append( cmdYear );

    listExtra.setAutoDelete( TRUE );
    listExtra.append( cmdExtra1 );
    listExtra.append( cmdExtra2 );
    listExtra.append( cmdExtra3 );
    listExtra.append( cmdExtra4 );
    listExtra.append( cmdExtra5 );
    listExtra.append( cmdExtra6 );
    listExtra.append( cmdExtra7 );
}

void RepeatEntry::slotNoEnd( bool unused )
{
    // if the item was toggled, then go ahead and set it to the maximum date
    if ( unused ) {
	end.setYMD( 3000, 12, 31 );
	cmdEnd->setText( RepeatEntryBase::tr("No End Date") );
    }
    else {
	end = start;
	endDateChanged( end.toString() );
    }

}

void RepeatEntry::endDateChanged( int y, int m, int d )
{
    end.setYMD( y, m, d );
}

void RepeatEntry::endDateChanged( const QString &s )
{
    cmdEnd->setText( fixDateString( s ) );
}

void RepeatEntry::setupRepeatLabel( const QString &s )
{
    lblVar1->setText( s );
}

void RepeatEntry::setupRepeatLabel( int x )
{
    // change the spelling based on the value of x
    QString strVar2;

    if ( x > 1 )
	lblVar1->show();
    else
	lblVar1->hide();

    switch ( currInterval ) {
	case NONE:
	    break;
	case DAY:
	    if ( x > 1 )
		strVar2 = tr( "days" );
	    else
		strVar2 = tr( "day" );	
	    break;
	case WEEK:
	    if ( x > 1 )
		strVar2 = tr( "weeks" );
	    else
		strVar2 = tr( "week" );
	    break;
	case MONTH:
	    if ( x > 1 )
		strVar2 = RepeatEntryBase::tr( "months" );
	    else
		strVar2 = tr( "month" );
	    break;
	case YEAR:
	    if ( x > 1 )
		strVar2 = RepeatEntryBase::tr( "years" );
	    else
		strVar2 = tr( "year" );
	    break;
    }
    if ( !strVar2.isNull() )
	lblVar2->setText( strVar2 );
}

void RepeatEntry::showRepeatStuff()
{
    cmdEnd->show();
    chkNoEnd->show();
    lblFreq->show();
    lblEvery->show();
    lblFreq->show();
    spinFreq->show();
    lblEnd->show();
    lblRepeat->setText( RepeatEntryBase::tr("Every") );
}

void RepeatEntry::slotWeekLabel( int )
{
    QString str;
    QListIterator<QToolButton> it( listExtra );
    unsigned int i;
    unsigned int keepMe = start.dayOfWeek() - 1;
    bool bNeedCarriage = FALSE;
    // don't do something we'll regret!!!
    if ( currInterval != WEEK )
	return;
    QStringList list;
    for ( i = 0; *it; ++it, i++ ) {
	// a crazy check, if you are repeating weekly, the current day
	// must be selected!!!
	if ( i == keepMe && !( (*it)->isOn() ) )
	    (*it)->setOn( TRUE );
	if ( (*it)->isOn() )
	    list.append( dayLabel[i] );
    }
    QStringList::Iterator itStr;
    for ( i = 0, itStr = list.begin(); itStr != list.end(); ++itStr, i++ ) {
	if ( i == 3 )
	    bNeedCarriage = TRUE;
	else
	    bNeedCarriage = FALSE;
	if ( str.isNull() )
	    str = *itStr;
	else if ( i == list.count() - 1 ) {
	    if ( i < 2 )
		str += tr(" and ") + *itStr;
	    else {
		if ( bNeedCarriage )
		    str += tr( ",\nand " ) + *itStr;
		else
		    str += tr( ", and " ) + *itStr;
	    }
	} else {
	    if ( bNeedCarriage )
		str += ",\n" + *itStr;
	    else
		str += ", " + *itStr;
	}
    }
    str = str.prepend( "on " );
    lblWeekVar->setText( str );
}

void RepeatEntry::slotMonthLabel( int type )
{
    QString str;
    if ( currInterval != MONTH || type > 1 )
	return;
    if ( type == 1 )
	str = strMonthDateTemplate.arg( numberPlacing(start.day()) );
    else
	str = strMonthDayTemplate.arg( numberPlacing(week(start)))
	      .arg( dayLabel[start.dayOfWeek() - 1] );
    lblRepeat->setText( str );
}

int week( const QDate &start )
{
    // figure out the week...
    int stop = start.day(),
        sentinel = start.dayOfWeek(),
        dayOfWeek = QDate( start.year(), start.month(), 1 ).dayOfWeek(),
        week = 1,
	i;
    for ( i = 1; i < stop; i++ ) {
	if ( dayOfWeek++ == sentinel )
	    week++;
	if ( dayOfWeek > 7 )
	    dayOfWeek = 0;
    }
    return week;
}

static QString fixDateString( const QString &s )
{
    QStringList l = QStringList::split( ' ', s );
    return l[1] + " " + l[2] + " " + l[3].right( 2 );
}

static QString numberPlacing( int x )
{
    // I hope this works in other languages besides english...
    QString str = QString::number( x );
    switch ( x % 10 ) {
	case 1:
	    str += QWidget::tr( "st" );
	    break;
	case 2:
	    str += QWidget::tr( "nd" );
	    break;
	case 3:
	    str += QWidget::tr( "rd" );
	    break;
	default:
	    str += QWidget::tr( "th" );
	    break;
    }
    return str;
}
