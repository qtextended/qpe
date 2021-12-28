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
#include "batterystatus.h"
#include "battery.h"
#include <qpainter.h>
#include <qpushbutton.h> 
#include <qdrawutil.h> 


BatteryStatus::BatteryStatus(BatteryMeter *bm) : 
		    QWidget( 0, 0, WDestructiveClose), parent(bm)
{
    show();
    setCaption( "Battery Status" );
    QPushButton *pb = new QPushButton( "Dismiss", this );
    pb->move( 70, 220 );
    pb->show();
    connect( pb, SIGNAL( clicked() ), this, SLOT( close() ) );
    showMaximized();
}

BatteryStatus::~BatteryStatus()
{
}

void BatteryStatus::drawSegment( QPainter *p, const QRect &r, const QColor &topgrad, const QColor &botgrad, const QColor &highlight, int hightlight_height )
{
    int h1, h2, s1, s2, v1, v2, ng = r.height(), hy = ng*30/100, hh = hightlight_height;
    topgrad.hsv( &h1, &s1, &v1 );
    botgrad.hsv( &h2, &s2, &v2 );
    for ( int j = 0; j < hy-2; j++ ) {
	p->setPen( QColor( h1 + ((h2-h1)*j)/(ng-1), s1 + ((s2-s1)*j)/(ng-1),
		    v1 + ((v2-v1)*j)/(ng-1),  QColor::Hsv ) );
	p->drawLine( r.x(), r.top()+hy-2-j, r.x()+r.width(), r.top()+hy-2-j );
    }
    for ( int j = 0; j < hh; j++ ) {
	p->setPen( highlight );
	p->drawLine( r.x(), r.top()+hy-2+j, r.x()+r.width(), r.top()+hy-2+j );
    }
    for ( int j = 0; j < ng-hy-hh; j++ ) {
	p->setPen( QColor( h1 + ((h2-h1)*j)/(ng-1), s1 + ((s2-s1)*j)/(ng-1),
		    v1 + ((v2-v1)*j)/(ng-1),  QColor::Hsv ) );
	p->drawLine( r.x(), r.top()+hy+hh-2+j, r.x()+r.width(), r.top()+hy+hh-2+j );
    }
}

void BatteryStatus::paintEvent( QPaintEvent *pe )
{
    QPainter p(this);
    int ac_status, /*bat_status, bat_flag,*/ percent, seconds;
    parent->getStatus( ac_status, 0, 0, percent, seconds );
    QString text;
    text.sprintf( tr("Percentage battery remaining") + ": %i%%", percent );
    p.drawText( 10, 120, text );
    if (ac_status!=0xff) {
	if (ac_status==2)
	    p.drawText( 10, 150, tr("On backup power") );
	else if (ac_status)
	    p.drawText( 10, 150, tr("Power on-line") );
	else
	    p.drawText( 10, 150, tr("External power disconnected") );
    }
#if 0
    //iPAQ kernel bug: seconds is completely wrong. We can put this code back
    //when the kernel is fixed.
    text.sprintf( tr("Battery time remaining") + ": %im %02is", seconds / 60, seconds % 60 );
    p.drawText( 10, 180, text );
#endif
    QColor c;
    QColor darkc;
    QColor lightc;
    if ( ac_status == 0x00 ) {
	c = blue.light(120);
	darkc = c.dark(280);
	lightc = c.light(145);
    } else if ( ac_status == 0x01 ) {
	c = green.dark(130);
	darkc = c.dark(200);
	lightc = c.light(220);
    } else {
	c = red;
	darkc = c.dark(280);
	lightc = c.light(140);
    }
    percent *= 2;
    p.setPen( black );
    qDrawWinPanel( &p,   9, 30, 204, 39, colorGroup(), TRUE, NULL);
    qDrawWinPanel( &p, 212, 37,  12, 24, colorGroup(), TRUE, NULL); //&QBrush( black ) );
    drawSegment( &p, QRect( 10, 30, percent, 40 ), lightc, darkc, lightc.light(115), 6 );
    drawSegment( &p, QRect( 11 + percent, 30, 200 - percent, 40 ), white.light(80), black, white.light(90), 6 );
    drawSegment( &p, QRect( 212, 37, 10, 25 ), white.light(80), black, white.light(90), 2 );
}

