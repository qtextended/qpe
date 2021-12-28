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
#include "organizer.h"
#include <qdatetime.h>
#include <qbuffer.h>
#include <qimage.h>
#include <time.h>

static int get_uid()
{
    return -1 * time( 0 );
}



OrganizerBase::OrganizerBase()
{
    id = get_uid();
}

int OrganizerBase::uid() const
{
    return id;
}




Todo::Todo()
    : OrganizerBase(), hDate( FALSE ), dt( QDate::currentDate() ),
      complete( FALSE ), prior( 6 ), categ(), note()
{
}

Todo::~Todo()
{

}

void Todo::setDate( bool b )
{
    hDate = b;
}

void Todo::setDate( const QDate &dt )
{
    this->dt = dt;
}

bool Todo::hasDate() const
{
    return hDate;
}

QDate Todo::date() const
{
    return dt;
}

void Todo::setCompleted( bool b )
{
    complete = b;
}

bool Todo::isCompleted() const
{
    return complete;
}

void Todo::setPriority( int p )
{
    prior = p;
}

int Todo::priority() const
{
    return prior;
}

QString Todo::description() const
{
    // grab the current first line, at least forty chars or so, strip extra
    // white space and repeat
    return ( note.left(40).simplifyWhiteSpace() );
}

void Todo::setCategory( const QString &c )
{
    categ = c;
}

QString Todo::category() const
{
    return categ;
}

void Todo::setNotes( const QString &n )
{
    note = n;
}

QString Todo::notes() const
{
    return note;
}

static void write( QTextStream &ts, const QDate& dt )
{
    ts << "<date><year>" << dt.year() << "</year><month>" << dt.month() << "</month><day>" << dt.day() << "</day></date>";
}

static void write( QTextStream &ts, const QTime& dt )
{
    ts << "<time><hour>" << dt.hour() << "</hour><minute>" << dt.minute() << "</minute><second>" << dt.second() << "</second></time>";
}

static void write( QTextStream &ts, const QDateTime& dt )
{
    write(ts,dt.date());
    write(ts,dt.time());
}

static void write( QTextStream &ts, const Event::RepeatPattern &r )
{
    ts << "<rtype>";
    switch ( r.type ) {
	case Event::Daily:
	    ts << "Daily";
	    break;
	case Event::Weekly:
	    ts << "Weekly";
	    break;
	case Event::MonthlyDay:
	    ts << "MonthlyDay";
	    break;
	case Event::MonthlyDate:
	    ts << "MonthlyDate";
	    break;
	case Event::Yearly:
	    ts << "Yearly";
	    break;
	default:
	    // do nothing
	    break;
    }
    ts << "</rtype>";
    if ( r.days > 0 ) {
	ts << "<weekdays>";
	ts << static_cast<int>(r.days);
	ts << "</weekdays>";
    }
    if ( r.position != 0 ) {
	ts << "<position>";
	ts << r.position;
	ts << "</position>";
    }
    ts << "<freq>" ;
    ts << r.frequency;
    ts << "</freq>";
    ts << "<hasenddate>" << r.hasEndDate << "</hasenddate>";
    if ( r.hasEndDate ) {
	ts << "<enddate>";
	write( ts, r.endDate );
	ts << "</enddate>";
    }
    ts << "<created>";
    ts << r.createTime;
    ts << "</created>";
}

static Event::SoundType textSoundType( const QString& t )
{
    if ( t == "loud" ) return Event::Loud;
    return Event::Silent;
}
static QString soundTypeText(Event::SoundType aSound)
{
    if ( aSound >= Event::Loud ) return "loud";
    return "silent";
}

static void read( Node* n, Event::SoundType& sound )
{
    sound = Event::Silent;
    while ( n ) {
	if ( n->tagName() == "sound" )
	    sound = textSoundType(n->data());
	n = n->nextNode();
    }
}

static void read( Node* n, QDate& date )
{
    int y=0, m=0, d=0;
    while ( n ) {
	if ( n->tagName() == "year" )
	    y = n->data().toInt();
	else if ( n->tagName() == "month" )
	    m = n->data().toInt();
	else if ( n->tagName() == "day" )
	    d = n->data().toInt();
	n = n->nextNode();
    }
    date.setYMD( y, m, d );
}

static void read( Node* n, QTime& time )
{
    int h=0, m=0, s=0;
    while ( n ) {
	if ( n->tagName() == "hour" )
	    h = n->data().toInt();
	else if ( n->tagName() == "minute" )
	    m = n->data().toInt();
	else if ( n->tagName() == "second" )
	    s = n->data().toInt();
	n = n->nextNode();
    }
    time.setHMS( h, m, s );
}

static void read( Node* n, QDateTime& dt )
{
    while ( n ) {
	if ( n->tagName() == "date" ) {
	    QDate d;
	    read(n->firstChild(),d);
	    dt.setDate(d);
	} else if ( n->tagName() == "time" ) {
	    QTime t;
	    read(n->firstChild(),t);
	    dt.setTime(t);
	}
	n = n->nextNode();
    }
}

static void read( Node *n, Event::RepeatPattern &r )
{
    while ( n ) {
	if ( n->tagName() == "rtype" ) {
	    Event::RepeatType tmpType;
	    if ( n->data() == "Daily" )
		tmpType = Event::Daily;
	    else if ( n->data() == "Weekly" )
		tmpType = Event::Weekly;
	    else if ( n->data() == "MonthlyDay" )
		tmpType = Event::MonthlyDay;
	    else if ( n->data() == "MonthlyDate" )
		tmpType = Event::MonthlyDate;
	    else if ( n->data() == "Yearly" )
		tmpType = Event::Yearly;
	    else
		tmpType = Event::NoRepeat;
	    r.type = tmpType;
	} else if ( n->tagName() == "freq" )
	    r.frequency = n->data().toInt();
	else if ( n->tagName() == "weekdays" )
	    r.days = n->data().toInt();
	else if ( n->tagName() == "position" )
	    r.position = n->data().toInt();
	else if ( n->tagName() == "created" )
	    r.createTime = n->data().toInt();
	else if ( n->tagName() == "hasenddate" )
	    r.hasEndDate = n->data().toInt();
	else if ( n->tagName() == "enddate" ) {
	    QDate dt;
	    read( n, dt );
	    r.endDate = dt;
	}
	n = n->nextNode();
    }
}

/*
static QByteArray base64Decode(const QString& ba64)
{
    // ...
    return QByteArray();
}

static QString base64Encode(const QByteArray& ba)
{
    // ...
    return QString();
}
*/

void Todo::save( QTextStream &ts, const QString &indent )
{
    ts << indent << "<HasDate>" << (int)hDate << "</HasDate>" << endl;
    ts << indent << "<Date>" << dt.toString() << "</Date>" << endl;
    ts << indent << "<Completed>" << (int)complete << "</Completed>" << endl;
    ts << indent << "<Priority>" << prior << "</Priority>" << endl;
    ts << indent << "<Category>" << escapeRichText(categ) << "</Category>" << endl;
    ts << indent << "<Description>" << escapeRichText(note) << "</Description>" << endl;
    ts << indent << "<Uid>" << uid() << "</Uid>" << endl;
}

void Todo::load( Node *n )
{
    while ( n ) {
	if ( n->tagName() == "HasDate" ) {
	    hDate = (bool)n->data().toInt();
	} else if ( n->tagName() == "Date" ) {
	    //dt = QDate::fromString( n->data() );
	} else if ( n->tagName() == "Completed" ) {
	    complete = (bool)n->data().toInt();
	} else if ( n->tagName() == "Priority" ) {
	    prior = n->data().toInt();
	} else if ( n->tagName() == "Category" ) {
	    categ = n->data();
	} else if ( n->tagName() == "Description" ) {
	    note = n->data() ;
	} else if ( n->tagName() == "Uid" ) {
	    setUid( n->data().toInt() );
	}
	n = n->nextNode();
    }
}

// --------------------------------------------------------------

Event::Event()
{
    startDate = endDate = QDateTime::currentDateTime();
    typ = Normal;
    hAlarm = FALSE;
    hRepeat = FALSE;
    aMinutes = 0;
    aSound = Silent;
    pattern.type = NoRepeat;
    pattern.frequency = -1;
}

Event::~Event()
{
}

void Event::setUid( int i )
{
    mUid = i;
}

int Event::uid() const
{
    return mUid;
}

void Event::setDescription( const QString &s )
{
    descript = s;
}

void Event::setLocation( const QString &s )
{
    locat = s;
}

void Event::setCategory( const QString &s )
{
    categ = s;
}

void Event::setType( Type t )
{
    typ = t;
}

void Event::setStart( const QDateTime &d )
{
    startDate = d;
}

void Event::setEnd( const QDateTime &d )
{
    endDate = d;
}

void Event::setTimeZone( const QString &z )
{
    tz = z;
}

void Event::setAlarm( bool b, int minutes, SoundType s )
{
    hAlarm = b;
    aMinutes = minutes;
    aSound = s;
}

void Event::setRepeat( bool b, const RepeatPattern &p )
{
    hRepeat = b;
    pattern = p;
}

void Event::setNotes( const QString &n )
{
    note = n;
}

QString Event::description() const
{
    return descript;
}

QString Event::location() const
{
    return locat;
}

QString Event::category() const
{
    return categ;
}

Event::Type Event::type() const
{
    return typ;
}

QDateTime Event::start( bool actual ) const
{
    QDateTime dt = startDate;
    if ( actual && typ == AllDay ) {
	QTime t = dt.time();
	t.setHMS( 0, 0, 0 );
	dt.setTime( t );
    }
    return dt;
}

QDateTime Event::end( bool actual ) const
{
    QDateTime dt = endDate;
    if ( actual && typ == AllDay ) {
	QTime t = dt.time();
	t.setHMS( 23, 59, 59 );
	dt.setTime( t );
    }
    return dt;
}

QString Event::timeZone() const
{
    return tz;
}

bool Event::hasAlarm() const
{
    return hAlarm;
}

int Event::alarmTime() const
{
    return aMinutes;
}

Event::SoundType Event::alarmSound() const
{
    return aSound;
}

bool Event::doRepeat() const
{
    return hRepeat;
}

Event::RepeatPattern Event::repeatPattern() const
{
    return pattern;
}

QString Event::notes() const
{
    return note;
}

bool Event::operator==( const Event &e ) const
{
    return ( e.descript == descript &&
	     e.locat == locat &&
	     e.categ == categ &&
	     e.typ == typ &&
	     e.startDate == startDate &&
	     e.endDate == endDate &&
	     e.tz == tz &&
	     e.hAlarm == hAlarm &&
	     e.aMinutes == aMinutes &&
	     e.aSound == aSound &&
	     e.hRepeat == hRepeat &&
	     e.pattern == pattern &&
	     e.note == note );
}

void Event::save( QTextStream &ts, const QString &indent )
{
    ts << indent << "<description>" << escapeRichText(descript) << "</description>" << endl;
    ts << indent << "<location>" << escapeRichText(locat) << "</location>" << endl;
    ts << indent << "<category>" << escapeRichText(categ) << "</category>" << endl;
    ts << indent << "<uid>" << uid() << "</uid>" << endl;
    if ( typ != Normal ) {
	ts << indent << "<type>";
	switch ( typ ) {
	    case Normal:
		break;
	    case AllDay:
		ts << "AllDay";
	}
	ts << "</type>" << endl;
    }
    if ( hAlarm ) {
	ts << indent << "<alarm>" << aMinutes;
	if ( aSound != Silent )
	    ts << "<sound>" << soundTypeText(aSound) << "</sound>";
	ts << "</alarm>" << endl;
    }
    if ( hRepeat ) {
	ts << indent << "<repeat>";
	write( ts, pattern );
	ts << "</repeat>" << endl;
    }
    ts << indent << "<start>";
	write(ts,startDate);
	ts << "</start>" << endl;
    if ( startDate != endDate ) {
	ts << indent << "<end>";
	    if ( startDate.date() != endDate.date() )
		write(ts,endDate.date());
	    write(ts,endDate.time());
	    ts << "</end>" << endl;
    }
    if ( !tz.isEmpty() )
	ts << indent << "<tz>" << tz << "</tz>";
    if ( !note.isEmpty() ) {
	ts << indent << "<note>" << escapeRichText(note) << "</note>" << endl;
    }
}

void Event::load( Node *n )
{
    bool e=FALSE;
    while ( n ) {
	if ( n->tagName() == "description" ) {
	    descript = n->data();
	} else if ( n->tagName() == "start" ) {
	    read(n->firstChild(), startDate);
	    if ( !e ) endDate = startDate;
	} else if ( n->tagName() == "end" ) {
	    read(n->firstChild(), endDate);
	    e=TRUE;
	} else if ( n->tagName() == "tz" ) {
	    tz = n->data();
	} else if ( n->tagName() == "location" ) {
	    locat = n->data();
	} else if ( n->tagName() == "category" ) {
	    categ = n->data();
	} else if ( n->tagName() == "type" ) {
	    if ( n->data() == "AllDay" )
		typ = AllDay;
	} else if ( n->tagName() == "alarm" ) {
	    aMinutes = n->data().toInt();
	    read(n->firstChild(), aSound);
	    hAlarm = TRUE;
	} else if ( n->tagName() == "repeat" ) {
	    hRepeat = TRUE;
	    read( n->firstChild(), pattern );
	} else if ( n->tagName() == "note" ) {
	    note = n->data();
	} else if ( n->tagName() == "uid" ) {
	    setUid( n->data().toInt() );
	}
	n = n->nextNode();
    }
}

EffectiveEvent::EffectiveEvent()
{
    mDate = QDate::currentDate();
    mStart = mEnd = QTime::currentTime();
}

EffectiveEvent::EffectiveEvent( Event e, const QDate &date )
{
    mEvent = e;

    mDate = date;
}

EffectiveEvent::~EffectiveEvent()
{
}

QString EffectiveEvent::category() const
{
    return mEvent.category();
}

QString EffectiveEvent::description( ) const
{
    return mEvent.description();
}

QString EffectiveEvent::location( ) const
{
    return mEvent.location();
}

QString EffectiveEvent::notes() const
{
    return mEvent.notes();
}

Event EffectiveEvent::event() const
{
    return mEvent;
}

QTime EffectiveEvent::end() const
{
    return mEnd;
}

QTime EffectiveEvent::start() const
{
    return mStart;
}

QDate EffectiveEvent::date() const
{
    return mDate;
}

int EffectiveEvent::length() const
{
    return (mEnd.hour() * 60 - mStart.hour() * 60)
	   + QABS(mStart.minute() - mEnd.minute() );
}

void EffectiveEvent::setDate( const QDate &dt )
{
    mDate = dt;
}

void EffectiveEvent::setStart( const QTime &start )
{
    mStart = start;
}

void EffectiveEvent::setEnd( const QTime &end )
{
    mEnd = end;
}

void EffectiveEvent::setEvent( Event e )
{
    mEvent = e;
}

bool EffectiveEvent::operator<( const EffectiveEvent &e ) const
{
    if ( mDate < e.date() )
	return TRUE;
    if ( mDate == e.date() )
	return ( mStart < e.start() );
    else
	return FALSE;
}

bool EffectiveEvent::operator<=( const EffectiveEvent &e ) const
{
    return (mDate <= e.date() );
}

bool EffectiveEvent::operator==( const EffectiveEvent &e ) const
{
    return ( mDate == e.date()
	     && mStart == e.start()
	     && mEnd == e.end()
	     && mEvent == e.event() );
}

bool EffectiveEvent::operator!=( const EffectiveEvent &e ) const
{
    return !(*this == e);
}

bool EffectiveEvent::operator>( const EffectiveEvent &e ) const
{
    return !(*this <= e );
}

bool EffectiveEvent::operator>=(const EffectiveEvent &e) const
{
    return !(*this < e);
}
