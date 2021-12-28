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
#include <qbutton.h>
#include <qpainter.h>
#include <resource.h>
#include "audiowidget.h"


static const int xo = -2; // movable x offset
static const int yo = 20; // movable y offset


struct MediaButton {
    int  xPos, yPos;
    bool isToggle, isBig, isHeld, isDown;
};


// Layout information for the buttons (and if it is a toggle button or not)
MediaButton buttons[] = {
    { 3*30-15+xo, 3*30-13+yo, FALSE,  TRUE, FALSE, FALSE }, // play
    {    1*30+xo,    5*30+yo, FALSE, FALSE, FALSE, FALSE }, // stop
    {    5*30+xo,    5*30+yo,  TRUE, FALSE, FALSE, FALSE }, // pause
    {  6*30-5+xo,    3*30+yo, FALSE, FALSE, FALSE, FALSE }, // next
    {  0*30+5+xo,    3*30+yo, FALSE, FALSE, FALSE, FALSE }, // previous
    {    3*30+xo,  0*30+5+yo, FALSE, FALSE, FALSE, FALSE }, // volume down
    {    3*30+xo,  6*30-5+yo, FALSE, FALSE, FALSE, FALSE }, // volume up
    {    5*30+xo,    1*30+yo,  TRUE, FALSE, FALSE, FALSE }, // repeat
    {    1*30+xo,    1*30+yo, FALSE, FALSE, FALSE, FALSE }  // playlist
};


static const int numButtons = (sizeof(buttons)/sizeof(MediaButton));


AudioWidget::AudioWidget(QWidget* parent, const char* name, WFlags f) :
    QWidget( parent, name, f )
{
    setBackgroundPixmap( Resource::loadPixmap( "mpegplayer/metalFinish" ) );
    pixmaps[0] = new QPixmap( Resource::loadPixmap( "mpegplayer/mediaButton1a" ) );
    pixmaps[1] = new QPixmap( Resource::loadPixmap( "mpegplayer/mediaButton2a" ) );
    pixmaps[2] = new QPixmap( Resource::loadPixmap( "mpegplayer/mediaButton1b" ) );
    pixmaps[3] = new QPixmap( Resource::loadPixmap( "mpegplayer/mediaButton2b" ) );
    pixmaps[4] = new QPixmap( Resource::loadPixmap( "mpegplayer/mediaControls" ) );
}


AudioWidget::~AudioWidget() {
    for ( int i = 0; i < 5; i++ )
	delete pixmaps[i];
}


void AudioWidget::paintButton( QPainter *p, int i ) {
    int x = buttons[i].xPos;
    int y = buttons[i].yPos;
    int offset = 22 + 14 * buttons[i].isBig + buttons[i].isDown;
    p->drawPixmap( x, y, *pixmaps[2 * buttons[i].isDown + buttons[i].isBig] );
    p->drawPixmap( x + offset, y + offset, *pixmaps[4], 18 * i, 0, 18, 18 );
}


void AudioWidget::mouseMoveEvent( QMouseEvent *event ) {
    for ( int i = 0; i < numButtons; i++ ) {
	int size = buttons[i].isBig;
	int x = buttons[i].xPos;
	int y = buttons[i].yPos;
	if ( event->state() == QMouseEvent::LeftButton ) {
	    QRect r( x, y, 64 + 22*size, 64 + 22*size );
	    if ( r.contains( event->pos() ) && !buttons[i].isHeld ) {
		buttons[i].isHeld = TRUE;
		buttons[i].isDown = !buttons[i].isDown;
		QPainter p(this);
		paintButton( &p, i );
		switch (i) {
		    case 0: emit playClicked(); return;
                    case 1: emit stopClicked(); return;
                    case 2: emit holdToggled(buttons[i].isDown); return;
                    case 3: emit nextClicked(); return;
		    case 4: emit prevClicked(); return;
                    case 5: emit moreClicked(); return;
                    case 6: emit lessClicked(); return;
                    case 7: emit loopToggled(buttons[i].isDown); return;
		    case 8: emit listClicked(); return;
		}
	    }
	} else {
	    if ( buttons[i].isHeld ) {
		buttons[i].isHeld = FALSE;
		if ( !buttons[i].isToggle ) {
		    buttons[i].isDown = FALSE;
		    QPainter p(this);
		    paintButton( &p, i );
		    if ( i == 5 )
			emit moreReleased();
		    else if ( 1 == 6 )
			emit lessReleased();
		    return;
		}
	    }
	}
    }
}


void AudioWidget::mousePressEvent( QMouseEvent *event ) {
    mouseMoveEvent( event );
}


void AudioWidget::mouseReleaseEvent( QMouseEvent *event ) {
    mouseMoveEvent( event );
}


void AudioWidget::showEvent( QShowEvent* ) {
    QMouseEvent event( QEvent::MouseMove, QPoint( 0, 0 ), 0, 0 );
    mouseMoveEvent( &event );
}


void AudioWidget::paintEvent( QPaintEvent *pe ) {
    QPainter p( this );

    // draw background
    // p.drawTiledPixmap( rect(), Resource::loadPixmap( "mpegplayer/metalFinish" ) );

    for ( int i = 0; i < numButtons; i++ )
	paintButton( &p, i );
}


