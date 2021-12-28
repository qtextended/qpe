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
#ifndef DATEBOOKMONTH
#define DATEBOOKMONTH

#include <qvbox.h>
#include <qhbox.h>
#include <qdatetime.h>
#include <qvaluelist.h>
#include <qtable.h>
#include "organizer.h"
#include "calendar.h"

class QToolButton;
class QComboBox;
class QSpinBox;
class Event;
class DateBookDB;

class DateBookMonthHeaderPrivate;
class DateBookMonthHeader : public QHBox
{
    Q_OBJECT

public:
    DateBookMonthHeader( QWidget *parent = 0, const char *name = 0 );
    ~DateBookMonthHeader();
    void setDate( int year, int month );

signals:
    void dateChanged( int year, int month );

private slots:
    void updateDate();
    void firstMonth();
    void lastMonth();
    void monthBack();
    void monthForward();

private:
    QToolButton *begin, *back, *next, *end;
    QComboBox *month;
    QSpinBox *year;
    DateBookMonthHeaderPrivate *d;
};

class DayItemMonthPrivate;
class DayItemMonth : public QTableItem
{
public:
    DayItemMonth( QTable *table, EditType et, const QString &t );
    ~DayItemMonth();
    void paint( QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected );
    void setDay( int d ) { dy = d; }
    void setEvents( const QValueList<Event> &events ) { daysEvents = events; };
    void setEvents( const QValueList<EffectiveEvent> &effEvents );
    void clearEvents() { daysEvents.clear(); };
    void clearEffEvents();
    int day() const { return dy; }
    void setType( Calendar::Day::Type t );
    Calendar::Day::Type type() const { return typ; }

private:
    QBrush back;
    QColor forg;
    int dy;
    Calendar::Day::Type typ;
    QValueList<Event> daysEvents; // not used anymore...
    DayItemMonthPrivate *d;
};

class DateBookMonthTablePrivate;
class DateBookMonthTable : public QTable
{
    Q_OBJECT

public:
    DateBookMonthTable( QWidget *parent = 0, const char *name = 0,
                        DateBookDB *newDb = 0 );
    ~DateBookMonthTable();
    void setDate( int y, int m, int d ) { selYear = year = y; selMonth = month = m; selDay = day = d; setupTable(); }

    QSize minimumSizeHint() const { return sizeHint(); }
    QSize minimumSize() const { return sizeHint(); }
    void  getDate( int& y, int &m, int &d ) const {y=selYear;m=selMonth;d=selDay;}
signals:
    void dateClicked( int year, int month, int day );

protected:
    void viewportMouseReleaseEvent( QMouseEvent * );

private slots:
    void dayClicked( int row, int col );
    void dragDay( int row, int col );

private:
    void setupTable();
    void findDay( int day, int &row, int &col );
    void getEvents();
    void changeDaySelection( int row, int col );

    int year, month, day;
    int selYear, selMonth, selDay;
    QValueList<Event> monthsEvents; // not used anymore...
    DateBookDB *db;
    DateBookMonthTablePrivate *d;
};

class DateBookMonthPrivate;
class DateBookMonth : public QVBox
{
    Q_OBJECT

public:
    DateBookMonth( QWidget *parent = 0, const char *name = 0, bool ac = FALSE,
                   DateBookDB *data = 0 );
    ~DateBookMonth();
    QDate  selectedDate() const;

signals:
    void dateClicked( int year, int month, int day );
    void dateClicked( const QString &s );

public slots:
    void setDate( int y, int m );
    void setDate( int y, int m, int d );

private slots:
    void forwardDateClicked( int y, int m, int d ) { emit dateClicked( QDate( y, m, d ).toString() ); }

private:
    DateBookMonthHeader *header;
    DateBookMonthTable *table;
    int year, month, day;
    bool autoClose;
    class DateBookMonthPrivate *d;
};

#endif
