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
#ifndef MPEGVIEW_H
#define MPEGVIEW_H


#include <qwidget.h>
#include <qdatetime.h>
#include "libmpeg3plugin.h"


// typedef struct mpeg3_rec mpeg3_t;


class MpegView : public QWidget {
    Q_OBJECT
public:
    MpegView( QWidget* parent=0, const char* name=0, WFlags fl=0 );
    ~MpegView();

    long currentFrame() const { return current_frame; }
    long totalFrames() const { return total_video_frames; }

    QSize sizeHint() const;
    bool hasVideo() const { return hasVideoChannel; }
    bool hasAudio() const { return hasAudioChannel; }
    bool isPaused() const { return paused; }
    
public slots:
    void setCurrentFrame(long);
    void setCurrentFrame(int);

    bool init(const QString& filename);
    void play();
    void playAgain();
    void stop();
    void setPaused( bool );
    void setRotation( bool );
    void setMute( bool );
    void setScaled( bool );
    
signals:
    void clicked();
    void frameDone(long);
    void playFinished();

protected:
    void timerEvent(QTimerEvent*);
    void mouseReleaseEvent(QMouseEvent* event);
private:
    LibMpeg3Plugin  *decoder;

//    void openAudio();
    bool playAudio();

    QTime playtime;
    int audioPauseTime;
    int audioSampleCounter;
    mpeg3_t *mpeg;
    long current_frame;
    long total_video_frames;
    long total_audio_samples;
    
    float framerate;
    int frameskip;
    int freq;
    int timerid;
    int stream;
    int framecount;
    int channels;

    bool hasVideoChannel;
    bool hasAudioChannel;
//    bool isLooping;
    bool isRotated;
    bool isMuted;
    bool dontSkip;
    bool oneone;
    bool paused;
    QString fileName;
};


#endif

