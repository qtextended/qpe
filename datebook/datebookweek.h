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
** $Id: datebookweek.h,v 1.17 2001/08/31 01:32:49 martinj Exp $
**
**********************************************************************/
#ifndef DATEBOOKWEEK
#define DATEBOOKWEEK

#include <qlist.h>
#include <qscrollview.h>
#include <qstring.h>
#include <qvaluelist.h>

#include <organizer.h>

class DateBookDB;
class DateBookWeekHeader;
class QDate;
class QLabel;
class QResizeEvent;
class QSpinBox;
class QTimer;
class QHeader;

class DateBookWeekItem
{
public:
    DateBookWeekItem( const EffectiveEvent e );

    void setGeometry( int x, int y, int w, int h );
    QRect geometry() const { return r; }

    const QColor &color() const { return c; }
    const EffectiveEvent event() const { return ev; }

private:
    const EffectiveEvent ev;
    QRect r;
    QColor c;
};

class DateBookWeekView : public QScrollView
{
    Q_OBJECT
public:
    DateBookWeekView( bool ampm, QWidget *parent = 0, const char *name = 0 );

    bool whichClock() const;
    void showEvents( QValueList<EffectiveEvent> &ev );
    void moveToHour( int h );

signals:
    void showDay( int d );
    void signalShowEvent( const EffectiveEvent & );
    void signalHideEvent();

private slots:
    void slotChangeClock( bool );

private:
    void positionItem( DateBookWeekItem *i );
    DateBookWeekItem *intersects( const DateBookWeekItem * );
    void drawContents( QPainter *p, int cx, int cy, int cw, int ch );
    void contentsMousePressEvent( QMouseEvent * );
    void contentsMouseReleaseEvent( QMouseEvent * );
    void resizeEvent( QResizeEvent * );

private:
    bool ampm;
    QHeader *header;
    QList<DateBookWeekItem> items;
    int rowHeight;
    bool showingEvent;
};

class DateBookWeek : public QWidget
{
    Q_OBJECT

public:
    DateBookWeek( bool ampm, DateBookDB *newDB, QWidget *parent = 0, const char *name = 0 );
    void setDate( int y, int w );
    QDate date() const;
    DateBookWeekView *weekView() const { return view; }
    void setStartViewTime( int startHere );
    int startViewTime() const;
    int week() const { return _week; };

public slots:
    void redraw();

signals:
    void showDate( int y, int m, int d );

private slots:
    void showDay( int day );
    void dateChanged( int y, int w );
    void slotShowEvent( const EffectiveEvent & );
    void slotHideEvent();

private:
    void getEvents();
    int year,
        _week;
    DateBookWeekHeader *header;
    DateBookWeekView *view;
    DateBookDB *db;
    QLabel *lblDesc;
    QTimer *tHide;
    int startTime;
    bool ampm;
};


#endif
