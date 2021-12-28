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
#ifndef AUDIODEVICE_H
#define AUDIODEVICE_H


#include <qobject.h>


class AudioDevicePrivate;


class AudioDevice : public QObject {
    Q_OBJECT
public:
    AudioDevice( unsigned int freq = 44000, unsigned int channels = 2, unsigned int bytesPerSample = 2 );
    ~AudioDevice();

    unsigned int canWrite() const;
    void write( char *buffer, unsigned int length );
    int bytesWritten();

    unsigned int channels() const;
    unsigned int frequency() const;
    unsigned int bytesPerSample() const;
    unsigned int bufferSize() const;

    // Each volume level is from 0x0000 to 0xFFFF
    static void getVolume( unsigned int& left, unsigned int& right );
    static void setVolume( unsigned int  left, unsigned int  right );
    static unsigned int getLeftVolume()  { unsigned int l, r; getVolume( l, r ); return l; }
    static unsigned int getRightVolume() { unsigned int l, r; getVolume( l, r ); return r; }
    static void increaseVolume() { setVolume( getLeftVolume() + 2000, getRightVolume() + 2000 ); }
    static void decreaseVolume() { setVolume( getLeftVolume() - 2000, getRightVolume() - 2000 ); }

public slots:
    // Convinence functions derived from above functions
    void setVolume( unsigned int level ) { setVolume( level, level ); }
    void mute() { setVolume( 0 ); }

signals:
    void completedIO();
    void volumeChanged( unsigned int );

private:
    AudioDevicePrivate *d;
};


#endif // AUDIODEVICE_H
