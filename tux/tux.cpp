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
#include "tux.h"
#include <resource.h>
#include <qwidget.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qfile.h> 

Tux::Tux( QWidget *parent=0, const char *name=0, WFlags f = 0) :
    QWidget(parent,name, f|
		    Qt::WStyle_Customize|Qt::WStyle_NoBorder)
{
    init(Resource::loadPixmap( "tux" ));
}

Tux::Tux( const QString& filename, QWidget *parent=0, const char *name=0, WFlags f = 0) :
    QWidget(parent,name, f|
		    Qt::WStyle_Customize|Qt::WStyle_NoBorder)
{
    if ( filename.isEmpty() )
	init(Resource::loadPixmap( "tux" ));
    else
	init(QPixmap(filename));
}

void Tux::init( QPixmap p )
{
    setBackgroundPixmap( p );
    setFixedSize( p.size() );
    if ( p.mask() )
	setMask( *p.mask() );
}

void Tux::mousePressEvent( QMouseEvent *e )
{
    clickPos = e->pos();
    movement = 0;
}

void Tux::mouseMoveEvent( QMouseEvent *e )
{
    QPoint newpos =  e->globalPos() - clickPos;
    movement += (pos()-newpos).manhattanLength();
    move( newpos );
}

void Tux::mouseReleaseEvent( QMouseEvent * )
{
    if ( movement < 4 )
	close();
}
