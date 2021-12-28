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

#include <qsound.h>
#include <qfile.h>
#include "resource.h"
#include "sound.h"

#ifndef QT_NO_SOUND
static int WAVsoundDuration(const QString& filename)
{
    // bad solution

    // most of this is copied from qsoundqss.cpp

    QFile input(filename);
    if ( !input.open(IO_ReadOnly) )
	return 0;

    struct QRiffChunk {
	char id[4];
	Q_UINT32 size;
	char data[4/*size*/];
    } chunk;

    struct {
        Q_INT16 formatTag;
        Q_INT16 channels;
        Q_INT32 samplesPerSec;
        Q_INT32 avgBytesPerSec;
        Q_INT16 blockAlign;
        Q_INT16 wBitsPerSample;
    } chunkdata;

    int total = 0;

    while(1) {
	// Keep reading chunks...
	const int n = sizeof(chunk)-sizeof(chunk.data);
	if ( input.readBlock((char*)&chunk,n) != n )
	    break;
	if ( qstrncmp(chunk.id,"data",4) == 0 ) {
	    total += chunkdata.avgBytesPerSec ?
		chunk.size * 1000 / chunkdata.avgBytesPerSec : 0;
//qDebug("%d bytes of PCM (%dms)", chunk.size,chunkdata.avgBytesPerSec ?  chunk.size * 1000 / chunkdata.avgBytesPerSec : 0);
	    input.at(input.at()+chunk.size-4);
	} else if ( qstrncmp(chunk.id,"RIFF",4) == 0 ) {
	    char d[4];
	    if ( input.readBlock(d,4) != 4 )
		return 0;
	    if ( qstrncmp(d,"WAVE",4) != 0 ) {
		// skip
//qDebug("skip %.4s RIFF chunk",d);
		if ( chunk.size < 10000000 )
		    !input.at(input.at()+chunk.size-4);
	    }
	} else if ( qstrncmp(chunk.id,"fmt ",4) == 0 ) {
	    if ( input.readBlock((char*)&chunkdata,sizeof(chunkdata)) != sizeof(chunkdata) )
		return 0;
#define WAVE_FORMAT_PCM 1
	    if ( chunkdata.formatTag != WAVE_FORMAT_PCM ) {
		//qDebug("WAV file: UNSUPPORTED FORMAT %d",chunkdata.formatTag);
		return 0;
	    }
	} else {
//qDebug("skip %.4s chunk",chunk.id);
	    // ignored chunk
	    if ( chunk.size < 10000000 )
		!input.at(input.at()+chunk.size);
	}
    }

//qDebug("%dms",total);
    return total;
}

class SoundData : public QSound {
public:
    SoundData(const QString& name) :
	QSound(Resource::findSound(name)),
	filename(Resource::findSound(name))
    {
    }

    void playLoop()
    {
	// needs server support

	int ms = WAVsoundDuration(filename);
	if ( ms )
	    startTimer(ms > 50 ? ms-50 : 0); // 50 for latency
	play();
    }

    void timerEvent(QTimerEvent*)
    {
	play();
    }

private:
    QString filename;
};
#endif

Sound::Sound(const QString& name)
{
#ifndef QT_NO_SOUND
    d = new SoundData(name);
#endif
}

Sound::~Sound()
{
#ifndef QT_NO_SOUND
    delete d;
#endif
}

void Sound::play()
{
#ifndef QT_NO_SOUND
    d->killTimers();
    d->play();
#endif
}

void Sound::playLoop()
{
#ifndef QT_NO_SOUND
    d->killTimers();
    d->playLoop();
#endif
}

void Sound::stop()
{
#ifndef QT_NO_SOUND
    d->killTimers();
#endif
}

