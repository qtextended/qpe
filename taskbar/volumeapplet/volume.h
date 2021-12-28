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
#ifndef __VOLUME_APPLET_H__
#define __VOLUME_APPLET_H__


#include <qwidget.h>
#include <qpixmap.h>
#include <qguardedptr.h>

class VolumeApplet : public QWidget
{
    Q_OBJECT
public:
    VolumeApplet( QWidget *parent = 0 );
    ~VolumeApplet();
    bool isMute( ) { return muted; }
    int percent( ) { return volumePercent; }

public slots:
    void setVolume( int percent );
    void sliderMoved( int percent );
    void mute( bool );

private:
    void writeSystemVolume();
    void mouseReleaseEvent( QMouseEvent * );
    void paintEvent( QPaintEvent* );
    int volumePercent;
    bool muted;
    QPixmap volumePixmap;
    QGuardedPtr<QWidget> volumeControl;
};


#endif // __VOLUME_APPLET_H__

