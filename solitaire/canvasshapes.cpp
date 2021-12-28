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
#include <qpainter.h>
#include <qcanvas.h>
#include "canvasshapes.h"


CanvasRoundRect::CanvasRoundRect(int x, int y, QCanvas *canvas) :
	QCanvasRectangle( x, y, 23, 36, canvas)
{
    setZ(0);
    show();
}


void CanvasRoundRect::redraw()
{
    hide();
    show();
}


void CanvasRoundRect::drawShape(QPainter &p)
{
    p.drawRoundRect( (int)x(), (int)y(), 23, 36);
}


CanvasCircleOrCross::CanvasCircleOrCross(int x, int y, QCanvas *canvas) :
	QCanvasRectangle( x, y, 21, 21, canvas), circleShape(TRUE)
{
    show();
}


void CanvasCircleOrCross::redraw()
{
    hide();
    show();
}


void CanvasCircleOrCross::setCircle()
{
    circleShape = TRUE;
    redraw();
}


void CanvasCircleOrCross::setCross()
{
    circleShape = FALSE;
    redraw();
}


void CanvasCircleOrCross::drawShape(QPainter &p)
{
    int x1 = (int)x(), y1 = (int)y();
    // Green circle
    if (circleShape == TRUE) {
	p.setPen( QPen( QColor(0x10, 0xE0, 0x10), 1 ) );
	p.drawEllipse( x1 - 1, y1 - 1, 21, 21);
	p.drawEllipse( x1 - 1, y1 - 0, 21, 19);
	p.drawEllipse( x1 + 0, y1 + 0, 19, 19);
	p.drawEllipse( x1 + 1, y1 + 0, 17, 19);
	p.drawEllipse( x1 + 1, y1 + 1, 17, 17);
    // Red cross
    } else {
	p.setPen( QPen( QColor(0xE0, 0x10, 0x10), 5 ) );
	p.drawLine( x1, y1, x1 + 20, y1 + 20);
	p.drawLine( x1 + 20, y1, x1, y1 + 20);
    }
}

