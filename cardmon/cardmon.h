/**********************************************************************
** Copyright (C) 2001 Devin Butterfield.  All rights reserved.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
**********************************************************************/

#ifndef CARDMON_H
#define CARDMON_H

#include <qwidget.h>
#include <qpixmap.h>

class CardMonitor : public QWidget {
public:
    CardMonitor( QWidget *parent = 0 );
    ~CardMonitor();
    bool getStatus( void );

protected:
    void timerEvent( QTimerEvent * );
    void paintEvent( QPaintEvent* );
    void mouseReleaseEvent( QMouseEvent * );
private:
    QPixmap pm;
    bool cardIn;
    int timerId;
};

#endif // CARDMON_H

