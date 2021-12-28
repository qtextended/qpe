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
** $Id: tzselect.h,v 1.4 2001/06/21 02:37:32 paultvete Exp $
**
**********************************************************************/
#ifndef TZSELECT_H
#define TZSELECT_H

#include <qhbox.h>
#include <qcombobox.h>
#include <qstringlist.h>

class QToolButton;

// a function to load defaults in case there is no file
QStringList timezoneDefaults( void );

class TZCombo : public QComboBox
{
    Q_OBJECT
public:
    TZCombo( QWidget* parent, const char* name = 0 );
    ~TZCombo();

    QString currZone() const;
    void setCurrZone( const QString& id );

protected:
    void keyPressEvent( QKeyEvent *e );
    void mousePressEvent(QMouseEvent*e);
    void updateZones();

private:
    QStringList identifiers;
    QStringList extras;
};

class TimeZoneSelectorPrivate;
class TimeZoneSelector : public QHBox
{
    Q_OBJECT
public:
    TimeZoneSelector( QWidget* parent = 0, const char* name=0 );
    ~TimeZoneSelector();
    
    QString currentZone() const;
    void setCurrentZone( const QString& id );

signals:
    void signalNewTz( const QString& id );

private slots:
    void slotTzActive( int index );
    void slotExecute( void );

private:
    TZCombo *cmbTz;
    QToolButton *cmdTz;
    TimeZoneSelectorPrivate *d;
};

#endif
