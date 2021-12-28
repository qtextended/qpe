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
#include <qpixmap.h>
#include "graph.h"

void GraphData::clear()
{
    names.clear();
    values.resize(0);
}

void GraphData::addItem( const QString &name, int value )
{
    names.append( name );
    values.resize( values.size() + 1 );
    values[values.size()-1] = value;
}

Graph::Graph(QWidget *parent, const char *name, WFlags f )
    : QFrame( parent, name, f )
{
}

PieGraph::PieGraph(QWidget *parent, const char *name, WFlags f )
    : Graph( parent, name, f )
{
}

void PieGraph::drawContents( QPainter *p )
{
    int size = QMIN( contentsRect().width(), contentsRect().height() ) - 1;

    int total = 0;
    for ( unsigned i = 0; i < data->count(); i++ )
	total += data->value(i);

    int angle = 0;
    for ( unsigned i = 0; i < data->count(); i++ ) {
	int len;
	if ( i == data->count() - 1 || !total )
	    len = 5760 - angle;
	else
	    len = data->value(i) * 5760 / total;
	QColor col;
	col.setHsv( i * 360 / data->count(), 255, 255 );
	p->setBrush( col );
	p->drawPie ( contentsRect().x(), contentsRect().y(),
		     size, size, angle, len+32 );
	angle += len;
    }
}

BarGraph::BarGraph(QWidget *parent, const char *name, WFlags f )
    : Graph( parent, name, f )
{
    setMinimumHeight( 10 );
    setMaximumHeight( 45 );
}

void BarGraph::drawContents( QPainter *p )
{
    int h = contentsRect().height();
    int y = 0;

    int total = 0;
    for ( unsigned i = 0; i < data->count(); i++ )
	total += data->value(i);

    int pos = 0;
    for ( unsigned i = 0; i < data->count(); i++ ) {
	int len;
	if ( i == data->count() - 1 || !total )
	    len = contentsRect().width() - pos;
	else
	    len = data->value(i) * contentsRect().width() / total;
	QColor col;
	col.setHsv( i * 360 / data->count(), 255, 255 );
	drawSegment( p, QRect(contentsRect().x() + pos, y, len, h), col );
	pos += len;
    }
}

void BarGraph::drawSegment( QPainter *p, const QRect &r, const QColor &c )
{
    if ( QPixmap::defaultDepth() > 8 ) {
	QColor topgrad = c.light(170);
	QColor botgrad = c.dark();

	int h1, h2, s1, s2, v1, v2;
	topgrad.hsv( &h1, &s1, &v1 );
	botgrad.hsv( &h2, &s2, &v2 );
	int ng = r.height();
	for ( int j =0; j < ng; j++ ) {
	    p->setPen( QColor( h1 + ((h2-h1)*j)/(ng-1),
			s1 + ((s2-s1)*j)/(ng-1),
			v1 + ((v2-v1)*j)/(ng-1),  QColor::Hsv ) );
	    p->drawLine( r.x(), r.top()+j, r.x()+r.width(), r.top()+j );
	}
    } else {
	p->fillRect( r.x(), r.top(), r.width(), r.height(), c );
    }
}


GraphLegend::GraphLegend( QWidget *parent, const char *name, WFlags f )
    : QFrame( parent, name, f )
{
}

void GraphLegend::drawContents( QPainter *p )
{
    int total = 0;
    for ( unsigned i = 0; i < data->count(); i++ )
	total += data->value(i);

    int th = p->fontMetrics().height() + 2;
    int y = 0;
    int angle = 0;
    for ( unsigned i = 0; i < data->count(); i++ ) {
	int len;
	if ( i == data->count() - 1 )
	    len = 5760 - angle;
	else
	    len = data->value(i) * 5760 / total;
	QColor col;
	col.setHsv( i * 360 / data->count(), 255, 255 );
	p->setBrush( col );
	p->drawRect( 1, y+1, th - 2, th - 2 );
	p->drawText( th + 1, y + p->fontMetrics().ascent()+1, data->name(i) );
	angle += len;
	y += th;
    }
    setMinimumHeight( y );
}

QSize GraphLegend::sizeHint() const
{
    int th = fontMetrics().height() + 2;
    int maxw = 0;
    for ( unsigned i = 0; i < data->count(); i++ ) {
	int w = fontMetrics().width( data->name(i) );
	if ( w > maxw )
	    maxw = w;
    }
    return QSize( maxw, th * data->count() );
}

