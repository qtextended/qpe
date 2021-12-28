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
#ifndef BATTERY_H
#define BATTERY_H

#include <qwidget.h>
#include <qguardedptr.h>

class BatteryMeter : public QWidget {
public:
    BatteryMeter( QWidget *parent = 0 );
    ~BatteryMeter();
    QSize sizeHint() const;
    void getStatus( int &, int*, int*, int &, int & );

protected:
    void timerEvent( QTimerEvent * );
    void paintEvent( QPaintEvent* );
    void mouseReleaseEvent( QMouseEvent * );
    QGuardedPtr<QWidget> batteryView;
    bool haveApm;
    int ac_status, percent, seconds;
};

#endif
