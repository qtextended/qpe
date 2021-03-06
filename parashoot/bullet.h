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
#include <qtimer.h>
#include <math.h>
#include <qcanvas.h>
#include "sound.h"

class Bullet : public QObject, public QCanvasSprite
{
    Q_OBJECT
public:
    Bullet(QCanvas*, double angle, int cannonx, int cannony);
    ~Bullet();
    void setXY(double angle);
    void checkCollision();
    void advance(int phase);   
    int rtti() const;
    static int getShotCount();
    static void setShotCount(int amount);
    static void setLimit(int amount);
    static void setNobullets(int amount);

signals:
    void score(int);

private:
    double dx;
    double dy;
    int damage;
    Sound bang;
};
