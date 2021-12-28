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
** $Id: datebookdb.cpp,v 1.26 2001/08/29 10:29:53 twschulz Exp $
**
**********************************************************************/

#include "alarmserver.h"
#include "global.h"
#include "datebookdb.h"
#include "xmlreader.h"
#include <qfile.h>
#include <qstring.h>
#include <qtextcodec.h>
#include <qtextstream.h>
#include <qtl.h>

#include <cstdlib>

// Helper functions

static QString dateBookFilename()
{
    return Global::applicationFileName("datebook","datebook.xml");
}

static void addEventAlarm( const Event &ev )
{
    QDateTime alarm =ev.start().addSecs( -ev.alarmTime()*60 );
    if ( alarm > QDateTime::currentDateTime() ) {
	AlarmServer::addAlarm( alarm,
			       "QPE/Application/datebook",
			       "alarm(QDateTime,int)", ev.alarmTime() );
    }
}

static void delEventAlarm( const Event &ev )
{
    AlarmServer::deleteAlarm( ev.start().addSecs( -ev.alarmTime()*60 ),
	"QPE/Application/datebook",
	"alarm(QDateTime,int)", ev.alarmTime() );
}


DateBookDB::DateBookDB()
{
    init();
}

DateBookDB::~DateBookDB()
{
    save();
    eventList.clear();
    repeatEvents.clear();
}

QValueList<Event> DateBookDB::getEvents( const QDate &from, const QDate &to )
{
    QValueListConstIterator<Event> it;
    QDateTime dtTmp, dtEnd;
    QValueList<Event> tmpList;
    for (it = eventList.begin(); it != eventList.end(); ++it ) {
        dtTmp = (*it).start(TRUE);
	dtEnd = (*it).end(TRUE);

        if ( dtTmp.date() >= from && dtTmp.date() <= to ) {
	    Event e = *it;
	    if ( dtTmp.date() != dtEnd.date() )
		e.setEnd( QDateTime(dtTmp.date(), QTime(23, 59, 0)) );
            tmpList.append( e );
	}
	// we must also check for end date information...
	if ( dtEnd.date() != dtTmp.date() && dtEnd.date() >= from ) {
	    QDateTime dt = dtTmp.addDays( 1 );
	    dt.setTime( QTime(0, 0, 0) );
	    QDateTime dtStop;
	    if ( dtEnd > to ) {
		dtStop = to;
	    } else
		dtStop = dtEnd;
	    while ( dt <= dtStop ) {
		Event ev = *it;
		if ( dt >= from ) {
		    ev.setStart( QDateTime(dt.date(), QTime(0, 0, 0)) );
		    if ( dt.date() == dtEnd.date() )
			ev.setEnd( QDateTime(dt.date(), dtEnd.time()) );
		    else
			ev.setEnd( QDateTime(dt.date(), QTime(23, 59, 0)) );
		    tmpList.append( ev );
		}
		dt = dt.addDays( 1 );
	    }
	}
    }
    // check for repeating events...
    QDate itDate = from;
    Event repeat;
    while ( itDate <= to ) {
	for ( it = repeatEvents.begin(); it != repeatEvents.end(); ++it ) {
	    if ( nextRepeat( *it, itDate, repeat ) ) {
		tmpList.append( repeat );
	    }
	}
	itDate = itDate.addDays( 1 );
    }
    qHeapSort( tmpList );
    return tmpList;
}

QValueList<Event> DateBookDB::getEvents( const QDateTime &start )
{
    QValueList<Event> day = getEvents(start.date(),start.date());

    QValueListConstIterator<Event> it;
    QDateTime dtTmp;
    QValueList<Event> tmpList;
    for (it = day.begin(); it != day.end(); ++it ) {
        dtTmp = (*it).start(TRUE);
        if ( dtTmp == start )
            tmpList.append( *it );
    }
    return tmpList;
}


QValueList<EffectiveEvent> DateBookDB::getEffectiveEvents( const QDate &from,
							   const QDate &to )
{
    QValueList<EffectiveEvent> tmpList;
    QValueListIterator<Event> it;

    EffectiveEvent effEv;
    QDateTime dtTmp,
	      dtEnd;

    for (it = eventList.begin(); it != eventList.end(); ++it ) {
        dtTmp = (*it).start(TRUE);
	dtEnd = (*it).end(TRUE);

        if ( dtTmp.date() >= from && dtTmp.date() <= to ) {
	    Event tmpEv = *it;
	    effEv.setEvent(tmpEv);
	    effEv.setDate( dtTmp.date() );
	    effEv.setStart( dtTmp.time() );
	    if ( dtTmp.date() != dtEnd.date() )
		effEv.setEnd( QTime(23, 59, 0) );
	    else
		effEv.setEnd( dtEnd.time() );
            tmpList.append( effEv );
	}
	// we must also check for end date information...
	if ( dtEnd.date() != dtTmp.date() && dtEnd.date() >= from ) {
	    QDateTime dt = dtTmp.addDays( 1 );
	    dt.setTime( QTime(0, 0, 0) );
	    QDateTime dtStop;
	    if ( dtEnd > to ) {
		dtStop = to;
	    } else
		dtStop = dtEnd;
	    while ( dt <= dtStop ) {
		Event tmpEv = *it;
		effEv.setEvent( tmpEv );
		effEv.setDate( dt.date() );
		if ( dt >= from ) {
		    effEv.setStart( QTime(0, 0, 0) );
		    if ( dt.date() == dtEnd.date() )
			effEv.setEnd( dtEnd.time() );
		    else
			effEv.setEnd( QTime(23, 59, 59) );
		    tmpList.append( effEv );
		}
		dt = dt.addDays( 1 );
	    }
	}
    }
    // check for repeating events...
    QDate itDate = from;
    Event repeat;
    while ( itDate <= to ) {
	for ( it = repeatEvents.begin(); it != repeatEvents.end(); ++it ) {
	    if ( nextRepeat( *it, itDate, repeat ) ) {
		Event tmpEv = *it;
		dtTmp = repeat.start(TRUE);
		effEv.setEvent( tmpEv );
		effEv.setDate( dtTmp.date() );
		effEv.setStart( dtTmp.time() );
		effEv.setEnd( repeat.end(TRUE).time() );
		tmpList.append( effEv );
	    }
	}
	itDate = itDate.addDays( 1 );
    }

    qHeapSort( tmpList );
    return tmpList;
}

QValueList<EffectiveEvent> DateBookDB::getEffectiveEvents( const QDateTime &dt)
{
    QValueList<EffectiveEvent> day = getEffectiveEvents(dt.date(), dt.date());
    QValueListConstIterator<EffectiveEvent> it;
    QValueList<EffectiveEvent> tmpList;
    QDateTime dtTmp;

    for (it = day.begin(); it != day.end(); ++it ) {
        dtTmp = QDateTime( (*it).date(), (*it).start() );
        if ( dtTmp == dt )
            tmpList.append( *it );
    }
    return tmpList;
}
	

void DateBookDB::addEvent( const Event &ev, bool doalarm )
{
    if ( doalarm && ev.hasAlarm() )
	addEventAlarm( ev );
    if ( ev.doRepeat() )
	repeatEvents.append( ev );
    else
	eventList.append( ev );
}

void DateBookDB::editEvent( const Event &old, Event &editedEv )
{
    if ( old.hasAlarm() )
	delEventAlarm( old );
    if ( old.doRepeat() ) {
	if ( old.doRepeat() && editedEv.doRepeat() ) {
	    // assumption, when someone edits a repeating event, they
	    // want to change them all, maybe not perfect, but it works
	    // for the moment...
	    Event orig;
	    if ( origRepeat( old, orig ) ) {	// should work always...
//		editedEv.setStart( orig.start() );  // Why was this done?
		// we've got the original, no sense searching for it again...
		repeatEvents.remove( orig );
	    }
	} else
	    removeRepeat( old );
    } else
	eventList.remove( old );
    if ( editedEv.hasAlarm() )
	addEventAlarm( editedEv );
    if ( editedEv.doRepeat() )
	repeatEvents.append( editedEv );
    else
	eventList.append( editedEv );
}

void DateBookDB::removeEvent( const Event &ev )
{
    if ( ev.hasAlarm() )
	delEventAlarm( ev );
    if ( ev.doRepeat() )
	removeRepeat( ev );
    else
	eventList.remove( ev );
}

void DateBookDB::init()
{
    QFile f( dateBookFilename() );
    if ( !f.open( IO_ReadOnly ) )
        return;
    QTextStream ts( &f );
    QXmlInputSource inputsource( ts );
    QXmlSimpleReader reader;
    XmlHandler handler;

    reader.setFeature( "http://trolltech.com/xml/features/report-whitespace-only-CharData", FALSE );
    reader.setContentHandler( &handler );
    reader.parse( inputsource );

    Node *n = handler.firstNode();
    while ( n ) {
        if ( n->tagName() == "event" ) {
            Event e;
            e.load( n->firstChild() );
	    addEvent(e,FALSE);
            n = n->nextNode();
            continue;
        }
        if ( n->lastChild() ) {
            n = n->firstChild();
        } else {
            if ( !n->nextNode() && n->parentNode() )
                n = n->parentNode();
            n = n->nextNode();
        }
    }
    f.close();
}

bool DateBookDB::save()
{
    QValueListIterator<Event> it;
    QFile f( dateBookFilename() );
    if ( !f.open( IO_WriteOnly ) )
        return FALSE;
    QTextStream ts( &f );
    ts.setCodec( QTextCodec::codecForName( "UTF-8" ) );
    ts << "<!DOCTYPE DATEBOOK><DATEBOOK>" << endl;
    for ( it = eventList.begin(); it != eventList.end(); ++it ) {
        ts << "<event>" << endl;
        (*it).save( ts, " ");
        ts << "</event>" << endl;
    }
    for ( it = repeatEvents.begin(); it != repeatEvents.end(); ++it ) {
        ts << "<event>" << endl;
        (*it).save( ts, " ");
        ts << "</event>" << endl;
    }
    ts << "</DATEBOOK>" << endl;
    f.close();
    return TRUE;
}

bool DateBookDB::nextRepeat( const Event &e, const QDate &from, Event &repeat )
{
    // a very simple method for determining if an event should be repeated
    // or not, look at the type, compute the next repeat that occurs after
    // from and then setup repeat.  Return TRUE or FALSE based on whether or not
    // we hit a repeat.  Be very strict on the conditions that need to match,
    // before we even start looking, because looking for a repeat could
    // potentially be expensive..

    // easy checks, first are we too far in the future or too far in the past?
    QDate dtTmp = e.start().date();
    QDate dtIt;
    int freq,
        iYear,
	iMonth,
	iDay,
	iStartofWeek,
        currDay,
        j;
    char repeatDays,
         it;
    if ( (e.repeatPattern().hasEndDate && e.repeatPattern().endDate < from)
	 || dtTmp > from )
	return FALSE;
    switch ( e.repeatPattern().type ) {
	case Event::Daily:
	    freq = e.repeatPattern().frequency;
	    // the simple case of daily...
	    if ( freq == 1 ) {
		repeat = e;
		repeat.setStart( QDateTime(from, e.start().time()) );
		return TRUE;
	    }
	    if ( dtTmp.daysTo( from ) % freq == 0 ) {
		repeat = e;
		repeat.setStart( QDateTime(from, e.start().time()) );
		return TRUE;
	    }
	    return FALSE;
	    break;
	case Event::Weekly:
	    repeatDays = e.repeatPattern().days;
	    currDay = from.dayOfWeek();
	    // a quick and _dirty_ way to get the right value to compare.
	    for ( j = 1, it = 1; j < currDay; j++ )
		it = it << 1;
	    if ( !(it & repeatDays) )
		return FALSE;
	    iStartofWeek = currDay - 1;
	    dtIt = dtTmp.addDays( -iStartofWeek );
	    freq = e.repeatPattern().frequency;

	    for (; dtIt <= from; dtIt = dtIt.addDays( 7 * freq ) ) {
		if ( dtIt.daysTo( from ) <= 7 ) {
		    repeat = e;
		    repeat.setStart( QDateTime(from, e.start().time()) );
		    return TRUE;
		}
	    }
	    return FALSE;
	    break;
	case Event::MonthlyDay: {
	    if ( from.dayOfWeek() != dtTmp.dayOfWeek() )
		return FALSE;
	    // try to figure out if the position is correct and then compare...
	    int stop = from.day(),
	        sentinel = from.dayOfWeek(),
	        dayOfWeek = QDate( from.year(), from.month(), 1 ).dayOfWeek(),
	        week = 1,
		i;
	    for ( i = 1; i < stop; i++ ) {
		if ( dayOfWeek++ == sentinel )
		    week++;
		if ( dayOfWeek > 7 )
		    dayOfWeek = 0;
	    }
	    if ( week == e.repeatPattern().position )  {
		repeat = e;
		repeat.setStart( QDateTime(from, e.start().time()) );
		return TRUE;
	    }
	    return FALSE;
	}
	case Event::MonthlyDate:
	    if ( dtTmp.day() != from.day() )
		return FALSE;
	    // handle the easy case of every month, and then work on the more
	    // interesting case...
	    freq = e.repeatPattern().frequency;
	    if ( freq == 1 ) {
		repeat = e;
		repeat.setStart( QDateTime(from, e.start().time()) );
		return TRUE;
	    }
	    iMonth = dtTmp.month();
	    iYear = dtTmp.year();
	    while ( dtTmp <= from ) {
		if ( dtTmp == from )
		    return TRUE;
		iMonth += freq;
		if ( iMonth >= 12 ) {
		    iMonth %= 12;
		    iYear++;
		}
		dtTmp.setYMD( iYear, iMonth, dtTmp.day() );
	    }
	    return FALSE;
	    break;
	case Event::Yearly:
	    if ( from.month() != dtTmp.month()
		 || from.day() != dtTmp.day() )
		return FALSE;
	    // handle the easy yearly case and then work on the harder cases
	    freq = e.repeatPattern().frequency;
	    if ( freq == 1 ) {
		repeat = e;
		repeat.setStart( QDateTime( from, e.start().time()) );
		return TRUE;
	    }
	    iYear = dtTmp.year();
	    iMonth = dtTmp.month();
	    iDay = dtTmp.day();
	    while ( dtTmp <= from ) {
		if ( dtTmp == from ) {
		    repeat = e;
		    repeat.setStart( QDateTime(from, e.start().time()) );
		    return TRUE;
		}
		iYear += freq;
		dtTmp.setYMD( iYear, iMonth, iDay );
	    }
	    return FALSE;
	    break;
	default:
	    return FALSE;
    }
}

bool DateBookDB::removeRepeat( const Event &ev )
{
    time_t removeMe = ev.repeatPattern().createTime;
    QValueListIterator<Event> it;
    for ( it = repeatEvents.begin(); it != repeatEvents.end(); ++it ) {
	if ( removeMe == (*it).repeatPattern().createTime ) {
	    repeatEvents.remove( *it );
	    // best break, or we are going into undefined territory!
	    return TRUE;
	}
    }
    return FALSE;
}

bool DateBookDB::origRepeat( const Event &ev, Event &orig ) const
{
    time_t removeMe = ev.repeatPattern().createTime;
    QValueListConstIterator<Event> it;
    for ( it = repeatEvents.begin(); it != repeatEvents.end(); ++it ) {
	if ( removeMe == (*it).repeatPattern().createTime ) {
	    orig = (*it);
	    return TRUE;
	}
    }
    return FALSE;
}
