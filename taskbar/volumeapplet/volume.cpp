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
#include <qcheckbox.h>
#include <qslider.h>
#include <qlayout.h>
#include <qpixmap.h>
#include "resource.h"
#include "volume.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>


VolumeApplet::VolumeApplet( QWidget *parent ) : QWidget( parent )
{
    setFixedHeight( 18 );
    setFixedWidth( 14 );
    volumePixmap = Resource::loadPixmap( "volume" );
    muted = FALSE; // ### read from pref
    volumePercent = 50; // ### read from pref
    volumeControl = 0;
    writeSystemVolume();
}

VolumeApplet::~VolumeApplet()
{
    delete volumeControl;
}

void VolumeApplet::mouseReleaseEvent( QMouseEvent *mouseEvent )
{
    if ( volumeControl ) {
        if ( volumeControl->isVisible() ) {
            volumeControl->hide();
        } else {
	    volumeControl->raise();
	    volumeControl->show();
	    volumeControl->setActiveWindow();
	}
        return;
    }

    // Create a small volume control window to adjust the volume with
    QWidget *sv = new QWidget( 0, 0, WDestructiveClose | WStyle_StaysOnTop );
    QVBoxLayout *vbox = new QVBoxLayout( sv );
    QSlider *volumeSlider = new QSlider( sv );
    QCheckBox *muteBox = new QCheckBox( tr("Mute"), sv );
    QPoint curPos = mouseEvent->globalPos();
    volumeSlider->setRange( 0, 100 );
    volumeSlider->setValue( 100 - volumePercent );
    volumeSlider->setTickmarks( QSlider::Both );
    volumeSlider->setTickInterval( 20 );
    volumeSlider->setFocusPolicy( QWidget::NoFocus );
    muteBox->setFocusPolicy( QWidget::NoFocus );
    muteBox->setChecked( muted );
    vbox->setMargin( 3 );
    vbox->addWidget( volumeSlider, 0, Qt::AlignVCenter | Qt::AlignHCenter );
    vbox->addWidget( muteBox );
    connect( volumeSlider, SIGNAL( valueChanged( int ) ), this, SLOT( sliderMoved( int ) ) );
    connect( muteBox, SIGNAL( toggled( bool ) ), this, SLOT( mute( bool ) ) );
    sv->setCaption( tr("Volume") );
    sv->setFixedHeight( 100 );
    sv->setFixedWidth( 65 );
    sv->move( curPos.x() - 30, curPos.y() - 115 );
    sv->setFocusPolicy( QWidget::NoFocus );
    sv->show();
    volumeControl = sv;
}


void VolumeApplet::mute( bool toggled )
{
    muted = toggled;
    // clear if removing mute
    repaint( !toggled );
    writeSystemVolume();
}


void VolumeApplet::sliderMoved( int percent )
{
    setVolume( 100 - percent );
}


void VolumeApplet::setVolume( int percent )
{
    // clamp volume percent to be between 0 and 100
    volumePercent = (percent < 0) ? 0 : ((percent > 100) ? 100 : percent);
    // repaint just the little volume rectangle
    repaint( 2, height() - 3, width() - 4, 2, FALSE );
    writeSystemVolume();
}

void VolumeApplet::writeSystemVolume()
{
    int fd = 0, vol = 0;
    if ((fd = open("/dev/mixer", O_RDWR))>=0) {
        vol = muted ? 0 : volumePercent;
        // set both channels to same volume
        vol |= vol << 8;
        ioctl(fd, MIXER_WRITE(0), &vol);
        ::close(fd);
    } else {
	setEnabled(FALSE);
    }
}

void VolumeApplet::paintEvent( QPaintEvent* )
{
    QPainter p(this);

    if (volumePixmap.isNull())
        volumePixmap = Resource::loadPixmap( "volume" );
    p.drawPixmap( 0, 1, volumePixmap );
    p.setPen( darkGray );
    p.drawRect( 1, height() - 4, width() - 2, 4 );

    int pixelsWide = volumePercent * (width() - 4) / 100;
    p.fillRect( 2, height() - 3, pixelsWide, 2, red );
    p.fillRect( pixelsWide + 2, height() - 3, width() - 4 - pixelsWide, 2, lightGray );

    if ( muted ) {
	p.setPen( red );
	p.drawLine( 1, 2, width() - 2, height() - 5 );
	p.drawLine( 1, 3, width() - 2, height() - 4 );
	p.drawLine( width() - 2, 2, 1, height() - 5 );
	p.drawLine( width() - 2, 3, 1, height() - 4 );
    }
}


