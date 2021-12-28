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
#ifndef MEDIA_PLAYER_H
#define MEDIA_PLAYER_H


#include <qmainwindow.h>
#include <qframe.h>


class QWidgetStack;
class PlayListWidget;
class AudioWidget;
class MpegView;
class DocLnk;


class MediaPlayer : public QMainWindow {
    Q_OBJECT
public:
    MediaPlayer(QWidget* parent=0, const char* name=0, WFlags f=0);
    ~MediaPlayer();

private slots:
    void playClicked();
    void nextClicked();
    void prevClicked();
    void listClicked();
    void moreClicked();
    void lessClicked();
    void moreReleased();
    void lessReleased();
    void stopClicked();
    void loopToggled(bool);
    void holdToggled(bool);
    void fullscreenToggled(bool);
    void setDocument(const QString& fileref);

protected:
    void timerEvent( QTimerEvent *e );

private:
    void readConfig();
    void writeConfig() const;

    QWidgetStack	*stack;
    PlayListWidget	*playList;
    AudioWidget		*audioUI;
    MpegView		*videoUI;

    int			volumeDirection;
    bool		playing;
    bool		looping;
    bool		paused;
    bool		shuffled;
    bool		fullscreen;
    const DocLnk	*currentFile;
};


#endif // MEDIA_PLAYER_H
