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
#include <qframe.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qpopupmenu.h>
#include <qpainter.h>
#include <global.h>
#include <applnk.h>
#include <resource.h>
#include "mrulist.h"


QList<MRUList>	*MRUList::MRUListWidgets = NULL;
QList<AppLnk>	*MRUList::task = NULL;


MRUList::MRUList( QWidget *parent )
       : QFrame( parent ), selected(-1), oldsel(-1)
{
    setBackgroundMode( PaletteButton );
    if (!MRUListWidgets)
	MRUListWidgets = new QList<MRUList>;
    if (!task)
	task = new QList<AppLnk>;
    MRUListWidgets->append( this );
}


MRUList::~MRUList()
{
    if (MRUListWidgets)
	MRUListWidgets->remove( this );
    if (task)
	task->setAutoDelete( TRUE );
}


QSize MRUList::sizeHint() const
{
    return QSize( frameWidth(), 16 );
}


void MRUList::addTask( const AppLnk *appLnk )
{
    unsigned int i = 0;

    if ( !task )
	return;

    for ( ; i < task->count(); i++ ) {
	AppLnk *t = task->at(i);
	if ( t->exec() == appLnk->exec() ) {
	    if (i != 0) {
		task->remove();
		task->prepend( t );
	    }
	    for (unsigned i = 0; i < MRUListWidgets->count(); i++ )
		MRUListWidgets->at(i)->update();
	    return;
	}
    }

    AppLnk *t = new AppLnk( *appLnk );
    task->prepend( t );

    if ( task->count() > 6 ) {
	t = task->last();
	task->remove();
	Global::terminate(t);
	delete t;
    }

    for (unsigned i = 0; i < MRUListWidgets->count(); i++ )
	MRUListWidgets->at(i)->update();
}


void MRUList::mousePressEvent(QMouseEvent *e)
{
    selected = e->pos().x() / 15;
    if ( selected >= (int)task->count() )
	selected = -1;
    else
	repaint( FALSE );
}


void MRUList::mouseReleaseEvent(QMouseEvent *)
{
    if ( selected >= 0 ) {
	if ( parentWidget() )
	    if ( parentWidget()->isA( "QPopupMenu" ) )
		parentWidget()->hide();
	Global::execute( task->at(selected)->exec() );
	MRUList::addTask( task->at(selected) );
	selected = -1;
	oldsel = -1;
	update();
    }
}


void MRUList::paintEvent( QPaintEvent * )
{
    QPainter p( this );
    AppLnk *t;
    int x = 0;
    int y = (height() - 14) / 2;
    int i = 0;

    p.fillRect( 0, 0, width(), height(), colorGroup().background() );

    if ( task ) {
	QListIterator<AppLnk> it( *task );
	for ( ; it.current(); i++, ++it ) {
	    t = it.current();
	    if ( (int)i == selected )
		p.fillRect( x, y, 15, t->pixmap().height()+1, colorGroup().highlight() );
	    else if ( (int)i == oldsel )
		p.eraseRect( x, y, 15, t->pixmap().height()+1 );
	    p.drawPixmap( x, y, t->pixmap() );
	    x += 15;
	}
    }
}

