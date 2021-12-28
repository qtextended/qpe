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
#include <qwidget.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <resource.h>
#include "wait.h"


Wait *lastWaitObject = NULL;


Wait::Wait( QWidget *parent ) : QWidget( parent ),
	pm( Resource::loadPixmap( "wait" ) ), waiting( FALSE )
{
    setFixedSize( pm.size() );
    lastWaitObject = this;
}


Wait *Wait::getWaitObject()
{
    return lastWaitObject;
}


void Wait::setWaiting( bool w )
{
    waiting = w;
    repaint();
}


void Wait::paintEvent( QPaintEvent * )
{
    QPainter p( this );
    if ( waiting )
	p.drawPixmap( 0, 0, pm );
    else
	p.eraseRect( rect() );
}


