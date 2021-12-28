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
**********************************************************************/
#ifndef PICKBOARDPICKS_H
#define PICKBOARDPICKS_H

#include <qframe.h>
#include <qlist.h>

// Internal stuff...

class PickboardConfig;

class PickboardPicks : public QFrame {
    Q_OBJECT
public:
    PickboardPicks(QWidget* parent=0, const char* name=0, WFlags f=0);
    ~PickboardPicks();
    QSize sizeHint() const;
    void initialise();
    void setMode(int);
    int currentMode() const { return mode; }

    void mousePressEvent(QMouseEvent*);

    void emitKey( ushort scan, ushort uni, ushort mod ) {key(scan,uni,mod);}
    void resetState();

signals:
    void key( ushort scan, ushort uni, ushort mod );

public slots:
    void doMenu();

protected:
    void drawContents( QPainter * );
    void mouseDoubleClickEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);

protected:
    int mode;
    QList<PickboardConfig> configs;

private:
    PickboardConfig* config() { return configs.at(mode); }
};


#endif
