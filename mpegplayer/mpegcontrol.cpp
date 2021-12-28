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
#include "mpegcontrol.h"
#include <qslider.h>
#include <qpushbutton.h>

const int scale=16;

/* XPM */
static const char * play_xpm[] = {
"12 12 2 1",
".	c None",
"*	c #000000",
"............",
"............",
"..**........",
"..****......",
"..******....",
"..********..",
"..********..",
"..******....",
"..****......",
"..**........",
"............",
"............"};

MpegControl::MpegControl( QWidget* parent, const char* name, WFlags fl ) :
    QHBox(parent,name,fl)
{
    play = new QPushButton(this);
    play->setToggleButton(TRUE);
    play->setAutoDefault(FALSE);
    connect(play, SIGNAL(toggled(bool)), this, SIGNAL(playingChanged(bool)));
    play->setPixmap(QPixmap(play_xpm));

    slider = new QSlider(Horizontal,this);
    slider->setTickmarks(QSlider::Above);
    slider->setTickInterval(4);
    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged(int)));
}

MpegControl::~MpegControl()
{
}

void MpegControl::setTotalFrames(long n)
{
    slider->setRange(0,(int)n/scale);
}

void MpegControl::setCurrentFrame(long n)
{
    slider->setValue((int)n/scale);
}

void MpegControl::sliderChanged(int n)
{
    emit currentFrameChanged(n*scale);
}
