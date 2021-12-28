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
** $Id: datebook.h,v 1.18 2001/08/31 01:26:18 martinj Exp $
**
**********************************************************************/
#ifndef DATEBOOK_H
#define DATEBOOK_H

#include <qmainwindow.h>
#include "datebookdb.h"

class QAction;
class QWidgetStack;
class DateBookDay;
class DateBookWeek;
class DateBookMonth;
class Event;
class QDate;

class DateBook : public QMainWindow
{
    Q_OBJECT

public:
    DateBook( QWidget *parent = 0, const char *name = 0, WFlags f = 0 );
    ~DateBook();

    bool needGui() const { return gui; }

signals:
    void newEvent();

protected:
    void timerEvent( QTimerEvent *e );
    
private slots:
    void fileNew();
    void slotSettings();
    void slotToday();	// view today
    void changeClock( bool newClock );
    void appMessage(const QCString& msg, const QByteArray& data);

    void viewDay();
    void viewWeek();
    void viewMonth();

    void showDay( int y, int m, int d );

    void saveRequired();

    void editEvent( const Event &e );
    void removeEvent( const Event &e );

private:
    int calcWeek( const QDate &d ) const;
    void addEvent( const Event &e );
    void initDay();
    void initWeek();
    void initMonth();
    void loadSettings();
    void saveSettings();

private:
    DateBookDB *db;
    QWidgetStack *views;
    DateBookDay *dayView;
    DateBookWeek *weekView;
    DateBookMonth *monthView;
    QAction *dayAction, *weekAction, *monthAction;
    bool aPreset;    // have everything set to alarm?
    int presetTime;  // the standard time for the alarm
    int startTime;
    bool ampm;
    bool gui;
};

#endif
