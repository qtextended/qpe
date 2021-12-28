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
** $Id: timestring.h,v 1.1 2001/05/22 13:38:19 twschulz Exp $
**
**********************************************************************/

#ifndef _TIMESTRING_H_
#include <qdatetime.h>
#include <qstring.h>

// return a string with the time based on whether or not you want
// you want it in 12 hour form.   if whichClock is true, then return
// it in 12 hour (am/pm) form otherwise return it in 24 hour form
// in theory Qt 3,0 handles this better (hopefully obsoleteing this)
QString timeString( const QTime &t, bool whichClock = false );
QString shortTime( bool ampm = false );
#endif
