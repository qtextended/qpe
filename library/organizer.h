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
#ifndef ORGANIZER_H
#define ORGANIZER_H

#include <qvaluelist.h>
#include <qvector.h>
#include <qdatetime.h>
#include <qpixmap.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <ctime>

#include "xmlreader.h"

class OrganizerBase
{
public:
    OrganizerBase();
    int uid() const;
    void setUid( int i ) { id = i; }

private:
    int id;

};

class TodoPrivate;
class Todo : public OrganizerBase
{
public:
    Todo();
    ~Todo();

    void setDate( bool b );
    void setDate( const QDate &dt );
    bool hasDate() const;
    QDate date() const;

    void setCompleted( bool b );
    bool isCompleted() const;

    void setPriority( int p );
    int priority() const;

    void setDescription( const QString &d );
    QString description() const;

    void setCategory( const QString &c );
    QString category() const;

    void setNotes( const QString &n );
    QString notes() const;

    void save( QTextStream &ts, const QString &indent );
    void load( Node *n );

private:
    bool hDate;
    QDate dt;
    bool complete;
    int prior;
    QString categ;
    QString note;
    TodoPrivate *d;
};

class EventPrivate;
class Event
{
public:
    enum RepeatType { NoRepeat = -1, Daily, Weekly, MonthlyDay,
                      MonthlyDate, Yearly };
    enum Days { MON = 0x01, TUE = 0x02, WED = 0x04, THU = 0x08,
                FRI = 0x10, SAT = 0x20, SUN = 0x40 };
    struct RepeatPattern
    {
	RepeatPattern() { type = NoRepeat; frequency = -1; days = 0;  position = 0; createTime = -1; };
	bool operator==( const RepeatPattern & ) const { return TRUE; }
	RepeatType type;
	int frequency;
	int position;	// the posistion in the month (e.g. the first sunday, etc) positive, count from the front negative count from the end...
	char days;  // a mask for days OR in your days!
	bool hasEndDate;
	QDate endDate;
	time_t createTime;
    };

    Event();
    ~Event();


    enum Type { Normal, AllDay };
    enum SoundType { Silent, Loud };

    //bool operator==( const Event &e1) { return start() == e1.start(); } const;
    bool operator<( const Event &e1) const { return start() < e1.start(); };
    bool operator<=( const Event &e1 ) const { return start() <= e1.start(); };
    bool operator!=( const Event &e1 ) const { return !( *this == e1 ); };
    bool operator>( const Event &e1 ) const { return start() > e1.start(); };
    bool operator>=(const Event &e1 ) const { return start() >= e1.start(); };
    void setDescription( const QString &s );
    void setLocation( const QString &s );
    void setCategory( const QString &s );
    void setType( Type t );
    void setStart( const QDateTime &d );
    void setEnd( const QDateTime &e );
    void setTimeZone( const QString & );
    void setAlarm( bool b, int minutes, SoundType );
    void setRepeat( bool b, const RepeatPattern &p );
    void setNotes( const QString &n );
    void setUid( int i );

    QString description() const;
    QString location() const;
    QString category() const;
    Type type() const;
    QDateTime start( bool actual = FALSE ) const;
    QDateTime end( bool actual = FALSE ) const;
    QString timeZone() const;
    bool hasAlarm() const;
    int alarmTime() const;
    SoundType alarmSound() const;
    bool doRepeat() const;
    RepeatPattern repeatPattern() const;
    QString notes() const;
    int uid() const;

    bool operator==( const Event &e ) const;

    void save( QTextStream &ts, const QString &indent );
    void load( Node *n );

private:
    QString descript, locat, categ;
    Type typ;
    QDateTime startDate, endDate;
    QString tz;
    bool hAlarm, hRepeat;
    int aMinutes;
    SoundType aSound;
    RepeatPattern pattern;
    QString note;
    int mUid;
    EventPrivate *d;
};

// Since an event spans multiple day, it is better to have this
// class to represent a day instead of creating many
// dummy events...

class EffectiveEventPrivate;
class EffectiveEvent
{
public:
    EffectiveEvent();
    EffectiveEvent( Event event, const QDate &startDate );
    ~EffectiveEvent();

    bool operator<( const EffectiveEvent &e ) const;
    bool operator<=( const EffectiveEvent &e ) const;
    bool operator==( const EffectiveEvent &e ) const;
    bool operator!=( const EffectiveEvent &e ) const;
    bool operator>( const EffectiveEvent &e ) const;
    bool operator>= ( const EffectiveEvent &e ) const;
		    		
    void setStart( const QTime &start );
    void setEnd( const QTime &end );
    void setEvent( Event e );
    void setDate( const QDate &date );

    QString category() const;
    QString description() const;
    QString location() const;
    QString notes() const;
    Event event() const;
    QTime start() const;
    QTime end() const;
    QDate date() const;
    int length() const;


private:
    class EffectiveEventPrivate *d;
    Event mEvent;
    QDate mDate;
    QTime mStart,
	  mEnd;

};
#endif
