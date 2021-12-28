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
#ifndef AUDIO_WIDGET_H
#define AUDIO_WIDGET_H


#include <qwidget.h>


class QPixmap;


class AudioWidget : public QWidget {
    Q_OBJECT
public:
    AudioWidget( QWidget* parent=0, const char* name=0, WFlags f=0 );
    ~AudioWidget();

signals:
    void playClicked();
    void stopClicked();
    void prevClicked();
    void nextClicked();
    void listClicked();
    void moreClicked();
    void lessClicked();
    void moreReleased();
    void lessReleased();
    void loopToggled(bool);
    void holdToggled(bool);

protected:
    void paintEvent( QPaintEvent *pe );
    void showEvent( QShowEvent *se );
    void mouseMoveEvent( QMouseEvent *event );
    void mousePressEvent( QMouseEvent *event );
    void mouseReleaseEvent( QMouseEvent *event );

private:
    void paintButton( QPainter *p, int i );

public:
    QPixmap *pixmaps[5];
};


#endif // AUDIO_WIDGET_H

