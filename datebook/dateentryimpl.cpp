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
** $Id: dateentryimpl.cpp,v 1.32 2001/09/05 23:38:52 paultvete Exp $
**
**********************************************************************/
#include "dateentryimpl.h"
#include "repeatentry.h"

#include <datebookmonth.h>
#include <global.h>
#include <qpeapplication.h>
#include <tzselect.h>

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qpopupmenu.h>
#include <qscrollview.h>
#include <qspinbox.h>
#include <qtoolbutton.h>

/*
 *  Constructs a DateEntry which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */

DateEntry::DateEntry( const QDateTime &start, const QDateTime &end,
		      bool whichClock, QWidget* parent,  const char* name )
    : DateEntryBase( parent, name, TRUE ),
      ampm( whichClock )
{
    init();
    setDates(start,end);
    QObject::connect( qApp, SIGNAL( clockChanged( bool ) ),
                      this, SLOT( slotChangeClock( bool ) ) );
}

static void addOrPick( QComboBox* combo, const QString& t )
{
    for (int i=0; i<combo->count(); i++) {
	if ( combo->text(i) == t ) {
	    combo->setCurrentItem(i);
	    return;
	}
    }
    combo->setEditText(t);
}

DateEntry::DateEntry( const Event &event, bool whichClock,
                      QWidget* parent,  const char* name )
    : DateEntryBase( parent, name, TRUE ),
      ampm( whichClock )
{
    init();
    setDates(event.start(),event.end());
    addOrPick( comboCategory, event.category() );
    addOrPick( comboDescription, event.description() );
    addOrPick( comboLocation, event.location() );
    checkAlarm->setChecked( event.hasAlarm() );
    checkAllDay->setChecked( event.type() == Event::AllDay );
    timezone->setCurrentZone(event.timeZone());
    editNote->setText(event.notes());
    spinAlarm->setValue(event.alarmTime());
    if ( event.alarmSound() != Event::Silent )
	comboSound->setCurrentItem( 1 );
    if ( event.doRepeat() ) {
	rp = event.repeatPattern();
	cmdRepeat->setText( tr("Repeat...") );
    }
    setRepeatLabel();
}

void DateEntry::setDates( const QDateTime& s, const QDateTime& e )
{
    int shour,
        ehour;
    QString strStart,
            strEnd;
    startDate = s.date();
    endDate = e.date();
    startTime = s.time();
    endTime = e.time();
    startDateChanged( s.date().toString() );
    if ( ampm ) {
	shour = s.time().hour();
	ehour = e.time().hour();
	if ( shour >= 12 ) {
	    if ( shour > 12 )
		shour -= 12;
	    strStart.sprintf( "%d:%02d PM", shour, s.time().minute() );
	} else {
	    if ( shour == 0 )
		shour = 12;
	    strStart.sprintf( "%d:%02d AM", shour, s.time().minute() );
	}
	if ( ehour >= 12 ) {
	    if ( ehour > 12 )
		ehour -= 12;
	    strEnd.sprintf( "%d:%02d PM", ehour, s.time().minute() );
	} else {
	    if ( ehour == 0 )
		ehour = 12;
	    strEnd.sprintf( "%d:%02d AM", ehour, s.time().minute() );
	}
    } else {
	strStart.sprintf( "%02d:%02d", s.time().hour(), s.time().minute() );
	strEnd.sprintf( "%02d:%02d", e.time().hour(), e.time().minute() );
    }
    comboStart->setEditText( strStart );
    endDateChanged( e.date().toString() );
    comboEnd->setEditText( strEnd );
}

void DateEntry::init()
{
    comboDescription->setInsertionPolicy(QComboBox::AtCurrent);
    comboLocation->setInsertionPolicy(QComboBox::AtCurrent);
    comboCategory->setInsertionPolicy(QComboBox::AtCurrent);

    initCombos();
    DateBookMonth *picker = 0;
    QPopupMenu *m1 = new QPopupMenu( this );
    picker = new DateBookMonth( m1, 0, TRUE );
    m1->insertItem( picker );
    buttonStart->setPopup( m1 );
    buttonStart->setPopupDelay( 0 );
    startPicker = picker;
    connect( picker, SIGNAL( dateClicked( const QString & ) ),
	     this, SLOT( startDateChanged( const QString & ) ) );
    connect( picker, SIGNAL( dateClicked( int, int, int ) ),
	     this, SLOT( startDateChanged( int, int, int ) ) );

    //Let start button change both start and end dates
    connect( picker, SIGNAL( dateClicked( const QString & ) ),
	     this, SLOT( endDateChanged( const QString & ) ) );
    connect( picker, SIGNAL( dateClicked( int, int, int ) ),
	     this, SLOT( endDateChanged( int, int, int ) ) );

    QPopupMenu *m2 = new QPopupMenu( this );
    picker = new DateBookMonth( m2, 0, TRUE );
    m2->insertItem( picker );
    buttonEnd->setPopup( m2 );
    buttonEnd->setPopupDelay( 0 );
    endPicker = picker;
    connect( picker, SIGNAL( dateClicked( const QString & ) ),
	     this, SLOT( endDateChanged( const QString & ) ) );
    connect( picker, SIGNAL( dateClicked( int, int, int ) ),
	     this, SLOT( endDateChanged( int, int, int ) ) );
    comboCategory->clear();
    // ### pref.
    QStringList categories;
    categories.append(tr("Business"));
    categories.append(tr("Personal"));
    comboCategory->insertStringList( categories );
}

static QString fixDateString( const QString &s )
{
    QStringList l = QStringList::split( ' ', s );
    return l[1] + " " + l[2] + " " + l[3].right( 2 );
}

/*
 *  Destroys the object and frees any allocated resources
 */
DateEntry::~DateEntry()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 * public slot
 */
void DateEntry::endDateChanged( const QString &s )
{
    // ### EEEK!  A race condidtion???
    if ( endDate < startDate ) {
        endDate = startDate;
	buttonEnd->setText( fixDateString( endDate.toString() ) );
    } else {
        buttonEnd->setText( fixDateString( s ) );
    }
}

/*
 * public slot
 */
void DateEntry::endDateChanged( int y, int m, int d )
{
    endDate.setYMD( y, m, d );
}

static QTime parseTime( const QString& s, bool ampm )
{
    QTime tmpTime;
    QStringList l = QStringList::split( ':', s );
    if ( ampm ) {
	int i=0;
	while (i<int(l[1].length()) && l[1][i]>='0' && l[1][i]<='9')
	    i++;
	QString digits = l[1].left(i);
	if ( l[1].contains( "PM", FALSE ) ) {
	    int hour = l[0].toInt();
	    if ( hour != 12 )
		tmpTime.setHMS( hour + 12, digits.toInt(), 0 );
	    else
		l[1] = digits;
	} else {
	    int hour = l[0].toInt();
	    if ( hour == 12 )
		tmpTime.setHMS( 0, digits.toInt(), 0 );
	    else
		l[1] = digits;
	}
    }
    if ( tmpTime.isNull() )
	tmpTime.setHMS( l[ 0 ].toInt(), l[1].toInt(), 0 );
    return tmpTime;
}

/*
 * public slot
 */
void DateEntry::endTimeChanged( const QString &s )
{
    QTime tmpTime = parseTime(s,ampm);
    if ( endDate > startDate || tmpTime >= startTime ) {
        endTime = tmpTime;
    } else {
        endTime = startTime;
        comboEnd->setCurrentItem( comboStart->currentItem() );
    }
}

/*
 * public slot
 */
void DateEntry::startDateChanged( const QString &s )
{
    QString strDay = fixDateString( s );
    buttonStart->setText( strDay );
}
/*
 * public slot
 */
void DateEntry::startDateChanged( int y, int m, int d )
{
    startDate.setYMD( y, m, d );
}

/*
 * public slot
 */
void DateEntry::startTimeChanged( int index )
{
    startTime = parseTime(comboStart->text(index),ampm);
    changeEndCombo( index );
}
/*
 * public slot
 */
void DateEntry::typeChanged( const QString &s )
{
    bool b = s != "All Day";
    buttonStart->setEnabled( b );
    comboStart->setEnabled( b );
    comboEnd->setEnabled( b );
}
/*
 * public slot
 */
void DateEntry::changeEndCombo( int change )
{
    if ( change + 2 < comboEnd->count() )
        change += 2;
    comboEnd->setCurrentItem( change );
    endTimeChanged( comboEnd->currentText() );
}

void DateEntry::slotRepeat()
{
    // Work around for compiler Bug..
    RepeatEntry *e;
    if ( rp.type != Event::NoRepeat )
	e = new RepeatEntry( rp, startDate, this );
    else
	e = new RepeatEntry( startDate, this );

#if defined(Q_WS_QWS) || defined(_WS_QWS_)
    e->showMaximized();
#endif
    if ( e->exec() ) {
	rp = e->repeatPattern();
	setRepeatLabel();
    }
}

Event DateEntry::event()
{
    Event ev;
    Event::SoundType st;
    ev.setDescription( comboDescription->currentText() );
    ev.setLocation( comboLocation->currentText() );
    ev.setCategory( comboCategory->currentText() );
    ev.setType( checkAllDay->isChecked() ? Event::AllDay : Event::Normal );
    if ( startDate > endDate ) {
        QDate tmp = endDate;
        endDate = startDate;
        startDate = tmp;
    }
    if ( startTime > endTime && endDate == startDate ) {
        QTime tmp = endTime;
        endTime = startTime;
        startTime = tmp;
    }
    // don't set the time if theres no need too
    if ( ev.type() == Event::AllDay ) {
        startTime.setHMS( 0, 0, 0 );
        endTime.setHMS( 0, 0, 0 );
    }
    ev.setStart( QDateTime( startDate, startTime ) );
    ev.setEnd( QDateTime( endDate, endTime ) );
    ev.setTimeZone( timezone->currentZone() );
    // we only have one type of sound at the moment... LOUD!!!
    if ( comboSound->currentItem() != 0 )
	st = Event::Loud;
    else
	st = Event::Silent;
    ev.setAlarm( checkAlarm->isChecked(), spinAlarm->value(), st );
    if ( rp.type != Event::NoRepeat )
	ev.setRepeat( TRUE, rp );
    ev.setNotes( editNote->text() );
    return ev;
}

void DateEntry::setRepeatLabel()
{

    switch( rp.type ) {
	case Event::Daily:
	    cmdRepeat->setText( tr("Daily...") );
	    break;
	case Event::Weekly:
	    cmdRepeat->setText( tr("Weekly...") );
	    break;
	case Event::MonthlyDay:
	case Event::MonthlyDate:
	    cmdRepeat->setText( tr("Montly...") );
	    break;
	case Event::Yearly:
	    cmdRepeat->setText( tr("Yearly...") );
	    break;
	default:
	    cmdRepeat->setText( tr("No Repeat...") );
    }
}

void DateEntry::setAlarmEnabled( bool alarmPreset, int presetTime, Event::SoundType sound )
{
    checkAlarm->setChecked( alarmPreset );
    spinAlarm->setValue( presetTime );
    if ( sound != Event::Silent )
	comboSound->setCurrentItem( 1 );
    else
	comboSound->setCurrentItem( 0 );
}

void DateEntry::initCombos()
{
    comboStart->clear();
    comboEnd->clear();
    if ( ampm ) {
	for ( int i = 0; i < 24; i++ ) {
	    if ( i == 0 ) {
		comboStart->insertItem( "12:00 AM" );
		comboStart->insertItem( "12:30 AM" );
		comboEnd->insertItem( "12:00 AM" );
		comboEnd->insertItem( "12:30 AM" );
	    } else if ( i == 12 ) {
		comboStart->insertItem( "12:00 PM" );
		comboStart->insertItem( "12:30 PM" );
		comboEnd->insertItem( "12:00 PM" );
		comboEnd->insertItem( "12:30 PM" );
	    } else if ( i > 12 ) {
		comboStart->insertItem( QString::number( i - 12 ) + ":00 PM" );
		comboStart->insertItem( QString::number( i - 12 ) + ":30 PM" );
		comboEnd->insertItem( QString::number( i - 12 ) + ":00 PM" );
		comboEnd->insertItem( QString::number( i - 12 ) + ":30 PM" );
	    } else {
		comboStart->insertItem( QString::number( i) + ":00 AM" );
		comboStart->insertItem( QString::number( i ) + ":30 AM" );
		comboEnd->insertItem( QString::number( i ) + ":00 AM" );
		comboEnd->insertItem( QString::number( i ) + ":30 AM" );
	    }
	}
    } else {
	for ( int i = 0; i < 24; i++ ) {
	    if ( i < 10 ) {
		comboStart->insertItem( QString("0")
		                        + QString::number(i) + ":00" );
		comboStart->insertItem( QString("0")
		                        + QString::number(i) + ":30" );
		comboEnd->insertItem( QString("0")
		                      + QString::number(i) + ":00" );
		comboEnd->insertItem( QString("0")
		                      + QString::number(i) + ":30" );
	    } else {
		comboStart->insertItem( QString::number(i) + ":00" );
		comboStart->insertItem( QString::number(i) + ":30" );
		comboEnd->insertItem( QString::number(i) + ":00" );
		comboEnd->insertItem( QString::number(i) + ":30" );
	    }
	}
    }
}

void DateEntry::slotChangeClock( bool whichClock )
{
    ampm = whichClock;
    initCombos();
    setDates( QDateTime( startDate, startTime ), QDateTime( endDate, endTime ) );
}
