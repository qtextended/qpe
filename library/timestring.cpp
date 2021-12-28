/**********************************************************************
** Copyright (C) 2001 Trolltech AS.  All rights reserved.
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
** $Id: timestring.cpp,v 1.1 2001/05/22 13:38:19 twschulz Exp $
**
**********************************************************************/

#include "timestring.h"
#include <qobject.h>

QString timeString( const QTime &t, bool whichClock )
{
    if ( !whichClock ) {
	return t.toString();
    }
    // ### else the hard case that should disappear in Qt 3.0
    QString argString = "%4:%5:%6 %7";
    int hour = t.hour();
    QString strMin = QString::number( t.minute() );
    QString strSec = QString::number( t.second() );
    if ( hour > 12 )
	argString = argString.arg( hour - 12, 2 );
    else {
        if ( hour == 0 )
	    argString = argString.arg( 12 );
        else
	    argString = argString.arg( hour, 2 );
    }
    if ( t.minute() < 10 )
	strMin.prepend( "0" );
    if ( t.second() < 10 )
	strSec.prepend( "0" );
    argString = argString.arg( strMin ).arg( strSec );
    if ( hour >= 12 )
	argString = argString.arg( QObject::tr("PM") );
    else
	argString = argString.arg( QObject::tr("AM") );
    return argString;
}

QString shortTime( bool ampm )
{
    static const char* const day[] = {
	    QT_TR_NOOP( "Mon" ),
	    QT_TR_NOOP( "Tue" ),
	    QT_TR_NOOP( "Wed" ),
	    QT_TR_NOOP( "Thu" ),
	    QT_TR_NOOP( "Fri" ),
	    QT_TR_NOOP( "Sat" ),
	    QT_TR_NOOP( "Sun" )
    };
    // just create a shorter time String
    QDateTime dtTmp = QDateTime::currentDateTime();
    QString strTime;
    strTime = QObject::tr( day[dtTmp.date().dayOfWeek()-1] ) + " " +
    timeString( dtTmp.time(), ampm );
    return strTime;
}
