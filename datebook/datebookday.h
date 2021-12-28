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
** $Id: datebookday.h,v 1.15 2001/08/28 18:29:11 twschulz Exp $
**
**********************************************************************/
#ifndef DATEBOOKDAY_H
#define DATEBOOKDAY_H

#include <qdatetime.h>
#include <qtable.h>
#include <qvbox.h>
#include <qlist.h>

#include <organizer.h>

class DateBookDayHeader;
class DateBookDB;
class QDateTime;
class QMouseEvent;
class QPaintEvent;
class QResizeEvent;

class DateBookDayView : public QTable
{
    Q_OBJECT
public:
    DateBookDayView( bool hourClock, QWidget *parent, const char *name );
    bool whichClock() const;
signals:
    void sigColWidthChanged();
protected slots:
    void slotChangeClock( bool );
protected:
    void resizeEvent( QResizeEvent *e );
    void initHeader();
private:
    bool ampm;
};

class DateBookDay;
class DateBookDayWidget : public QWidget
{
    Q_OBJECT

public:
    DateBookDayWidget( const EffectiveEvent &e, DateBookDay *db );
    ~DateBookDayWidget();

    void setX( int x );
    void setWidth( int w );

    EffectiveEvent event() const { return ev; }

signals:
    void deleteMe( const Event &e );
    void editMe( const Event &e );

protected:
    void paintEvent( QPaintEvent *e );
    void mousePressEvent( QMouseEvent *e );

private:
    void place();

private:
    const EffectiveEvent ev;
    DateBookDay *dateBook;
    int xpos, wid;
    QString text;

};

class DateBookDay : public QVBox
{
    Q_OBJECT

public:
    DateBookDay( bool ampm, DateBookDB *newDb, QWidget *parent, const char *name );
    void selectedDates( QDateTime &start, QDateTime &end );
    QDate date() const;
    DateBookDayView *dayView() const { return view; }
    void setStartViewTime( int startHere );
    int startViewTime() const;

public slots:
    void setDate( int y, int m, int d );
    void redraw();

signals:
    void removeEvent( const Event& );
    void editEvent( const Event& );
    void newEvent();

private slots:
    void dateChanged( int y, int m, int d );
    void slotColWidthChanged() { relayoutPage( true ); };

private:
    void getEvents();
    void relayoutPage( bool fromResize = false );
    QDate currDate;
    DateBookDayView *view;
    DateBookDayHeader *header;
    DateBookDB *db;
    QList<DateBookDayWidget> widgetList;
    int startTime;
};

#endif
