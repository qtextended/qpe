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
** $Id: dateentryimpl.h,v 1.10 2001/08/23 07:06:05 warwick Exp $
**
**********************************************************************/
#ifndef DATEENTRY_H
#define DATEENTRY_H

#include "dateentry.h"
#include <organizer.h>
#include <qdatetime.h>

class DateBookMonth;

class DateEntry : public DateEntryBase
{
    Q_OBJECT

public:
    DateEntry( const QDateTime &start, const QDateTime &end,
	bool whichClock = FALSE, QWidget* parent = 0, const char* name = 0 );
    DateEntry( const Event &event, bool whichCLock = FALSE, QWidget* parent = 0,
               const char* name = 0 );
    ~DateEntry();

    Event event();
    void setAlarmEnabled( bool alarmPreset, int presetTime, Event::SoundType );

public slots:
    void endDateChanged( const QString & );
    void endDateChanged( int, int, int );
    void endTimeChanged( const QString & );
    void startDateChanged( const QString & );
    void startDateChanged(int, int, int);
    void startTimeChanged( int index );
    void typeChanged( const QString & );
    void changeEndCombo( int change );
    void slotRepeat();
    void slotChangeClock( bool );

private:
    void init();
    void initCombos();
    void setDates( const QDateTime& s, const QDateTime& e );
    void setRepeatLabel();

    DateBookMonth *startPicker, *endPicker;
    QDate startDate, endDate;
    QTime startTime, endTime;
    Event::RepeatPattern rp;
    bool ampm;
};

#endif // DATEENTRY_H
