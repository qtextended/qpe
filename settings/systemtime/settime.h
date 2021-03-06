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
#ifndef SYSTEM_TIME_H
#define SYSTEM_TIME_H


#include <qdatetime.h>
#include <qdialog.h>


class QToolButton;
class QSpinBox;
class QCheckBox;
class TimeZoneSelector;
class DateBookMonth;


class SetTime : public QWidget
{
    Q_OBJECT
public:
    SetTime( QWidget *parent=0, const char *name=0 );

    QTime time() const;

    bool viewAP( void ) const;

public slots:
    void slotTzChange( const QString& tz );

protected slots:
    void hourChanged( int value );
    void minuteChanged( int value );

protected:
    int hour;
    int minute;
    QCheckBox *ampm;
    QSpinBox *sbHour;
    QSpinBox *sbMin;
};


class SetDate : public QWidget
{
    Q_OBJECT
public:
    SetDate( QWidget *parent=0, const char *name=0 );

    QDate date() const { return m_date; }

public slots:
    void slotTzChange( const QString& tz );

protected slots:
    void dateClicked( int year, int month, int day );

protected:
    QDate m_date;
    DateBookMonth* dbm; 
};


class SetDateTime : public QDialog
{
    Q_OBJECT
public:
    SetDateTime( QWidget *parent=0, const char *name=0, bool modal=FALSE,  WFlags f=0 );

protected:
    virtual void accept();

    SetTime *time;
    SetDate *date;
    TimeZoneSelector *tz;
};


#endif

