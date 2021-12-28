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
#include "mpegview.h"
#include "audiodevice.h"
#include <qfile.h>
#include <qimage.h>
#include <qpainter.h>
#include "resource.h"
#ifdef Q_WS_QWS
#include <config.h>
#include <qcopenvelope_qws.h>
#include <qdirectpainter_qws.h>
#include <qgfxraster_qws.h>
#define USE_DIRECT_PAINTER
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static char	    *audioBuffer = NULL;
static AudioDevice *audioDevice = NULL;


static const int bytes_per_sample = 2; //16 bit


#ifdef QPIM_STANDALONE
# define TIME_DEBUG
#endif


// # define TIME_DEBUG


#ifdef TIME_DEBUG
static double audio_timestamp;
static double video_timestamp;
#define TIME_DEBUG_INFO(x)	qDebug x
#else
#define TIME_DEBUG_INFO(x)
#endif


/*
  Returns FALSE when the last piece of audio data has been fed to the
  device.
*/
bool MpegView::playAudio()
{
    int outputBytes;

    for ( int i = 0; i < 4 && (outputBytes = audioDevice->canWrite() ); i++) {
	bool err;
	if ( channels == 2 ) {
	    err = decoder->audioReadStereoSamples( (short*)audioBuffer, outputBytes / 4, 0 );
	} else {
	    err = decoder->audioReadSamples( (short*)audioBuffer, 0, outputBytes / 2, 0 );
	}
	if ( err ) {
	    qWarning( "Error reading audio data." );
	    return FALSE;
	}
	audioDevice->write( audioBuffer, outputBytes );
	audioSampleCounter += outputBytes;
    }

    return TRUE;
}


MpegView::MpegView( QWidget* parent, const char* name, WFlags fl ) :
    QWidget(parent,name,fl)
{
    isRotated = FALSE;
    isMuted = FALSE;
    oneone = TRUE;
    paused = FALSE;
    setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
    decoder = new LibMpeg3Plugin;
}


MpegView::~MpegView() {
    stop();
}


void MpegView::setPaused( bool pause ) {
    if ( !decoder->isOpen() )
	return;

    if ( pause ) {
	if ( timerid )
	    killTimer( timerid );
	timerid = 0;
    } else {
	if ( !timerid )
	    timerid = startTimer( 0 );
    }

    paused = pause;
}


void MpegView::stop() {

#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
    {
	QCopEnvelope("QPE/System", "setScreenSaverInterval(int)" ) << -1;
    }
#endif

    if ( decoder->isOpen() ) {

	decoder->close();

	if ( timerid )
	    killTimer(timerid);

	if ( audioDevice ) {
	    delete audioDevice;
	    delete audioBuffer;
	    audioDevice = 0;
	    audioBuffer = 0;
	}

    }
}


bool MpegView::init( const QString& filename ) {
    stop();
    dontSkip = isMuted;
    fileName = filename;
    stream = 0; // only play stream 0 for now
    current_frame = 0;
    frameskip = 0;
    total_video_frames = total_audio_samples = 0;
    
    QCString fn = QFile::encodeName(filename);

    if ( !decoder->isFileSupported( fn.data() ) )
	return FALSE;

    if ( !decoder->open( fn.data() ) )
	return FALSE;

    hasAudioChannel = decoder->audioStreams() > 0;
    hasVideoChannel = decoder->videoStreams() > 0;

    if ( hasAudioChannel ) {
	int astream = 0;

	channels = decoder->audioChannels( astream );
	TIME_DEBUG_INFO(( "channels = %d\n", channels ));
	
	total_audio_samples = decoder->audioSamples( astream );
	
	freq = decoder->audioFrequency( astream );
	TIME_DEBUG_INFO(( "frequency = %d\n", freq ));

	audioSampleCounter = 0;
	audioPauseTime = 0;

	audioDevice = new AudioDevice( freq, channels, bytes_per_sample );
	audioBuffer = new char[ audioDevice->bufferSize() ];
	channels = audioDevice->channels();

	//### must check which frequency is actually used.
	static const int size = 1;
	short int buf[size];
	decoder->audioReadSamples( buf, 0, size, stream );
	double first_timestamp = decoder->getTime();
	if ( first_timestamp > 0.5 )
	    audioPauseTime = int( first_timestamp*1000 );
	TIME_DEBUG_INFO(( "Initial audio timestamp %g", first_timestamp ));
#ifdef TIME_DEBUG
	audio_timestamp = first_timestamp;
#endif
	dontSkip = FALSE;
    } else {
	dontSkip = TRUE;
    }
    
    if ( hasVideoChannel ) {
	total_video_frames = decoder->videoFrames( stream );
	framerate = decoder->videoFrameRate( stream );
	TIME_DEBUG_INFO(( "Frame rate %g total %ld", framerate, total_video_frames ));
	if ( framerate <= 1.0 ) {
	    TIME_DEBUG_INFO(( "Crazy frame rate, resetting to sensible" ));
	    framerate = 25;
	}
	if ( total_video_frames == 1 ) {
	    TIME_DEBUG_INFO(( "Cannot seek to frame" ));
	    dontSkip = TRUE;
	}

	int mpegWidth = decoder->videoWidth( stream );
	if ( oneone )
	    isRotated = ( mpegWidth > qt_screen->width() ) ? TRUE : FALSE;
	else
	    isRotated = ( mpegWidth > decoder->videoHeight( stream ) ) ? TRUE : FALSE;
    }

    setBackgroundColor( black );
    // setBackgroundColor( green.dark() );

    return TRUE;
}


void MpegView::play() {

#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
{
    // Stop the screen from blanking and power saving state
    QCopEnvelope("QPE/System", "setScreenSaverInterval(int)" ) << 0;
}
#endif

    timerid = startTimer(0);
    playtime.start();
    updateGeometry();
}


void MpegView::setCurrentFrame( long f ) {
    current_frame = f;
    frameskip = 0;
    if ( hasVideoChannel ) {
	decoder->videoSetFrame( f, stream );
	playtime.start();
	playtime.addMSecs( -int(float(f)/framerate*1000) );
    }
}


void MpegView::setCurrentFrame( int f ) {
    setCurrentFrame((long)f);
}


void MpegView::mouseReleaseEvent(QMouseEvent* event) {
    if ( rect().contains(event->pos()) )
	emit clicked();
}


void MpegView::timerEvent(QTimerEvent* event) {
    if ( event->timerId() != timerid )
	return;

    killTimer(timerid);
    timerid = 0;

    bool more = FALSE;
    int delay = 0;

    // turning off mute is not likely to work too well...
    if ( hasAudioChannel && !isMuted ) {
	int bytesPerMilliSecond = bytes_per_sample * channels * freq / 1000;	// 176

	const int fudge = 40; // takes time to start playing and time between each timer event

        if ( playtime.elapsed() + fudge < audioPauseTime )
	    more = TRUE;
	else if ( playAudio() ) {
	    more = TRUE;

	    static bool outputDelay = FALSE;//TRUE;
	    
	    if ( outputDelay ) {
		int buffered;

		if ( (buffered = audioDevice->bytesWritten()) == -1 ) {
		    outputDelay = FALSE;
		    more = TRUE;
		}

		// Timestamp support in libmpeg3 is not good enough for synchronization
		int skew = playtime.elapsed() - (audioSampleCounter - buffered) / bytesPerMilliSecond - audioPauseTime;
	    
		if ( skew < -1000 || skew > 1000 )
		    playtime = playtime.addMSecs( skew );
		else 
		    playtime = playtime.addMSecs( skew / 5 );

#ifdef TIME_DEBUG
		audio_timestamp = decoder->getTime();
		TIME_DEBUG_INFO(( "audio_timestamp %g, elapsed %d, audioPauseTime %i, buffered %i ",
			audio_timestamp, playtime.elapsed(), audioPauseTime, buffered ));
#endif
	    }

            more = audioSampleCounter <= total_audio_samples * bytes_per_sample * channels;
	}

	delay = audioDevice->bufferSize() / bytesPerMilliSecond;
	TIME_DEBUG_INFO(( "delay: %i", delay ));
    } 
   
    if ( hasVideoChannel ) {
	int sw = decoder->videoWidth( stream );
	int sh = decoder->videoHeight( stream );

	int w;
	int h;

	if ( oneone ) {
	    w = sw;
	    h = sh;
	} else {
	    w = isRotated ? height() : width();
	    h = isRotated ? width() : height();
	    if ( w*sh > sw*h ) {
		w = sw*h/sh;
	    } else {
		h = sh*w/sw;
	    }
	}

	int dd = QPixmap::defaultDepth();

#ifdef TIME_DEBUG
	long set_frame = frameskip ? current_frame+frameskip : 0;
#endif

	if ( frameskip > 0 ) 
	    decoder->videoSetFrame( current_frame+frameskip, stream );

	int format = (dd == 16) ? MPEG3_RGB565 : MPEG3_BGRA8888;

	bool done = FALSE;

#ifdef USE_DIRECT_PAINTER
	QDirectPainter p(this);
	const int xf = qt_screen->transformOrientation();
	if ( ( isRotated && xf==3 || !isRotated && !xf ) && p.numRects()==1 ) {
	    w--; // we can't allow libmpeg to overwrite.
	    int ox, oy;
	    if ( isRotated ) {
		QPoint roff = qt_screen->mapToDevice(p.offset(),QSize(qt_screen->width(),qt_screen->height()));
		ox = roff.x()-height()+2 + (height()-w)/2;
		oy = roff.y() + (width()-h)/2;
	    } else {
		ox = p.xOffset()+(width()-w)/2;
		oy = p.yOffset()+(height()-h)/2;
	    }
	    QRect c = p.rect(0) & QRect(ox,oy,w,h);
	    int sx,sy;
	    if ( c != QRect(ox,oy,w,h) ) {
		// clip
		sx = (c.x()-ox) * sw / w;
		sy = (c.y()-oy) * sh / h;
		sw = c.width() * sw / w;
		sh = c.height() * sh / h;
		ox = c.x();
		oy = c.y();
		w = c.width();
		h = c.height();
	    } else {
		sx = sy = 0;
	    }
	    uchar* fp = p.frameBuffer() + p.lineStep()*oy;
	    fp += dd*ox/8;
	    uchar **jt = new uchar*[h];
	    for (int i=h; i; i-- ) {
		jt[h-i] = fp;
		fp += p.lineStep();
	    }
	    if ( decoder->videoReadScaledFrame( jt, sx, sy, sw, sh, w, h, format, 0) == 0 )
		done = TRUE;
	    delete [] jt;
	}
#endif

	if ( !done ) {
	    QImage frame( w + 2, h, (dd == 16) ? 16 : 32 );
	    if ( decoder->videoReadScaledFrame( frame.jumpTable(), 0, 0, sw, sh, w, h, format, 0) == 0 ) {
		QPainter p(this);
		if ( isRotated ) {
		    const int rw = frame.height();
		    const int rh = frame.width();
		    QImage rframe(frame.height(),frame.width(),frame.depth());
		    if ( rframe.depth() == 16 ) {
			ushort* in = (ushort*)frame.bits();
			ushort* out = (ushort*)rframe.bits();
			int spl = rframe.bytesPerLine()/2;
			for (int x=0; x<rw; x++) {
			    ushort* lout = out++ + (rh-1)*spl;
			    for (int y=0; y<rh; y++) {
				*lout=*in++;
				lout-=spl;
			    }
			}
		    } else {
			uint* in = (uint*)frame.bits();
			uint* out = (uint*)rframe.bits();
			int spl = rframe.bytesPerLine()/4;
			for (int x=0; x<rw; x++) {
			    uint* lout = out++ + (rh-1)*spl;
			    for (int y=0; y<rh; y++) {
				*lout=*in++;
				lout-=spl;
			    }
			}
		    }
		    int ox = (width()-rw)/2;
		    int oy = (height()-rh)/2;
		    p.drawImage(ox,oy,rframe,0,0,rw,rh);
		} else {
		    int ox = (width()-w)/2;
		    int oy = (height()-h)/2;
		    p.drawImage(ox,oy,frame,0,0,w,h);
		}
		done = TRUE;
	    }
	}

	if ( done ) {
#ifdef TIME_DEBUG
	    video_timestamp = decoder->getTime();
#endif
	    current_frame = decoder->videoGetFrame( stream );
	    emit frameDone( current_frame );
	    more = TRUE;
#ifdef TIME_DEBUG
	    if ( set_frame )
		qDebug( "Set to %ld, got %ld ", set_frame, current_frame );
#endif
	    int frametime = int(float(current_frame+frameskip)/framerate*1000);
	    delay = frametime - playtime.elapsed();

	    int offset = int(delay*framerate);

	    if ( frameskip || delay < 0 )
		delay = 0;

	    if ( !dontSkip && ((frameskip && offset > 1000) || offset < -1000 )) {
		if ( offset < 0 ) 
		    frameskip++;
		else
		    frameskip--;
		TIME_DEBUG_INFO(( "new frameskip %d (offset %d)", frameskip, offset ));
		//recalc delay using same code as above (###)
		frametime = int(float(current_frame+frameskip)/framerate*1000);
	    }
	    TIME_DEBUG_INFO(( "frametime %d, delay %d", frametime, delay ));
	}
	TIME_DEBUG_INFO(( " next video_timestamp %g   realtime %g", video_timestamp, float(playtime.elapsed())/1000 ));
    }

    if ( more ) {
	timerid = startTimer( delay );
    } else {
	emit playFinished();
    }
}


QSize MpegView::sizeHint() const {
    if ( decoder->isOpen() )
	return QSize( decoder->videoWidth( stream ), decoder->videoHeight( stream ) );
    else
	return QSize(40,30);
}


void MpegView::setRotation( bool on ) {
    isRotated = on;
    repaint();
}


void MpegView::playAgain() {
    if ( !fileName.isEmpty() ) {
	init( fileName );
	play();
    }
}


void MpegView::setMute( bool on ) {
    isMuted = on;
    if ( isMuted ) {
	frameskip = 0;
	dontSkip = TRUE;
    }
}


void MpegView::setScaled( bool on ) {
    oneone = !on;
    repaint();
}

