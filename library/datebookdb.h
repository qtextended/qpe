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
** $Id: datebookdb.h,v 1.10 2001/08/28 18:29:11 twschulz Exp $
**
**********************************************************************/

#ifndef DATEBOOKDB_H
#define DATEBOOKDB_H

#include <qdatetime.h>
#include <qvaluelist.h>
#include "organizer.h"
class DateBookDBPrivate;
class DateBookDB
{
public:
    DateBookDB();
    ~DateBookDB();

    // very depreciated now!!!
    QValueList<Event> getEvents( const QDate &from, const QDate &to );
    QValueList<Event> getEvents( const QDateTime &start );

    // USE THESE!!!
    QValueList<EffectiveEvent> getEffectiveEvents( const QDate &from,
						   const QDate &to );
    QValueList<EffectiveEvent> getEffectiveEvents( const QDateTime &start );

    void addEvent( const Event &ev, bool doalarm=TRUE );
    void removeEvent( const Event &ev );
    void editEvent( const Event &old, Event &ev );

    bool save();
private:
    // helper functions for repeats
    bool nextRepeat( const Event &e, const QDate &from, Event &repeat );
    //find the real repeat...
    bool origRepeat( const Event &ev, Event &orig ) const;
    bool removeRepeat( const Event &ev );
    void init();

    QValueList<Event> eventList; // non-repeating events...
    QValueList<Event> repeatEvents; // the repeating events...
    DateBookDBPrivate *d;
};

#endif
