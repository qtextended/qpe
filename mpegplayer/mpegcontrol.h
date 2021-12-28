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
#ifndef MPEGCONTROL_H
#define MPEGCONTROL_H

#include <qhbox.h>

class QSlider;
class QPushButton;

class MpegControl : public QHBox {
    Q_OBJECT
public:
    MpegControl( QWidget* parent=0, const char* name=0, WFlags fl=0 );
    ~MpegControl();

    void setTotalFrames(long);

signals:
    void currentFrameChanged(long);
    void playingChanged(bool);

public slots:
    void setCurrentFrame(long);

private slots:
    void sliderChanged(int);

private:
    QSlider* slider;
    QPushButton* play;
};

#endif
