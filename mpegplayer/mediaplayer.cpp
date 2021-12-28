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
#include <qmainwindow.h>
#include <qwidgetstack.h>
#include <qapplication.h>
#include <resource.h>
#include "mediaplayer.h"
#include "playlistwidget.h"
#include "audiowidget.h"
#include "mpegview.h"
#include "audiodevice.h"
#include <qsound.h>
#include <qfile.h>
#include <config.h>

#define MPEG_NO_CONTROL


MediaPlayer::MediaPlayer(QWidget* parent, const char* name, WFlags f) :
    QMainWindow(parent,name,f), volumeDirection( 0 ), playing( FALSE ),
    looping( FALSE ), paused( FALSE ), shuffled( FALSE ), fullscreen( FALSE ), currentFile( NULL )
{
#ifndef Q_WS_QWS
    setFixedSize( 240, 320 );
#endif

    stack    = new QWidgetStack(this);
    playList = new PlayListWidget( stack );
    videoUI  = new MpegView( stack );
    audioUI  = new AudioWidget( stack );

    setCentralWidget( stack );

    fullscreenToggled( TRUE );

    connect( playList, SIGNAL( playClicked() ),         this, SLOT( playClicked() ) );
    connect( playList, SIGNAL( stopClicked() ),         this, SLOT( stopClicked() ) );
    connect( playList, SIGNAL( prevClicked() ),         this, SLOT( prevClicked() ) );
    connect( playList, SIGNAL( nextClicked() ),         this, SLOT( nextClicked() ) );
    connect( playList, SIGNAL( setFullScreen(bool) ),   this, SLOT( fullscreenToggled(bool) ) );
    connect( playList, SIGNAL( setScaled(bool) ),    videoUI, SLOT( setScaled(bool) ) );

    connect( audioUI,  SIGNAL( playClicked() ),         this, SLOT( playClicked() ) );
    connect( audioUI,  SIGNAL( nextClicked() ),         this, SLOT( nextClicked() ) );
    connect( audioUI,  SIGNAL( prevClicked() ),         this, SLOT( prevClicked() ) );
    connect( audioUI,  SIGNAL( listClicked() ),         this, SLOT( listClicked() ) );

    connect( audioUI,  SIGNAL( moreClicked() ),         this, SLOT( moreClicked() ) );
    connect( audioUI,  SIGNAL( lessClicked() ),         this, SLOT( lessClicked() ) );
    connect( audioUI,  SIGNAL( moreReleased() ),        this, SLOT( moreReleased() ) );
    connect( audioUI,  SIGNAL( lessReleased() ),        this, SLOT( lessReleased() ) );

    connect( audioUI,  SIGNAL( stopClicked() ),         this, SLOT( stopClicked() ) );
    connect( audioUI,  SIGNAL( holdToggled(bool) ),     this, SLOT( holdToggled(bool) ) );
    connect( audioUI,  SIGNAL( loopToggled(bool) ), playList, SLOT( setLooping(bool) ) );

    connect( videoUI,  SIGNAL( playFinished() ),        this, SLOT( nextClicked() ) );
    connect( videoUI,  SIGNAL( clicked() ),		this, SLOT( listClicked() ) );

    stack->raiseWidget( playList );
//    statusBar();

    readConfig();
}


MediaPlayer::~MediaPlayer() {
    writeConfig();
}


void MediaPlayer::writeConfig() const {
    Config cfg( "MediaPlayer" );
    playList->writeConfig( cfg );
}


void MediaPlayer::readConfig() {
    Config cfg( "MediaPlayer" );
    playList->readConfig( cfg );
}


void MediaPlayer::setDocument(const QString& fileref)
{
    playList->setDocument( fileref );
    playClicked();
}


void MediaPlayer::playClicked() {

//    if ( currentFile != playList->current() ) {
	stopClicked();
	currentFile = playList->current();
//    }

    if ( ( currentFile == NULL ) || !QFile::exists( currentFile->file() ) ) {
	return;
    }

    if ( currentFile->type() == "audio/x-wav" ) {
	QSound::play( currentFile->file() );
    } else {
	if ( !paused && !playing ) {
	    if ( videoUI->init( currentFile->file() ) ) {
#ifndef MPEG_NO_CONTROL
		control->setCurrentFrame(0);
		control->setTotalFrames( mpeg->totalFrames() );
#endif
	    }
	}
	if ( videoUI->hasVideo() ) {
	    if ( fullscreen )
		videoUI->show();
	    else
		stack->raiseWidget( videoUI );
	} else {
	    if ( fullscreen )
		audioUI->show();
	    else
		stack->raiseWidget( audioUI );
	}

	if ( !paused && !playing )
	    videoUI->play();

	if ( !paused )
	    playing = TRUE;
    }
}


void MediaPlayer::stopClicked() {
    if ( playing )
	videoUI->stop();
    playing = FALSE;
}


void MediaPlayer::prevClicked() {
    if ( playList->prev() )
	playClicked();
    else if ( looping ) {
        if ( playList->last() )
	    playClicked();
    } else {
	stopClicked();
	stack->raiseWidget( playList );
	audioUI->hide();
	videoUI->hide();
    }
}


void MediaPlayer::nextClicked() {
    if ( playList->next() )
	playClicked();
    else if ( looping ) {
        if ( playList->first() )
	    playClicked();
    } else {
	stopClicked();
	stack->raiseWidget( playList );
	audioUI->hide();
	videoUI->hide();
    }
}


void MediaPlayer::listClicked() {
    stopClicked();
    stack->raiseWidget( playList );
    audioUI->hide();
    videoUI->hide();
}


void MediaPlayer::lessClicked() {
    volumeDirection = -1;
    startTimer( 100 );
}


void MediaPlayer::moreClicked() {
    volumeDirection = +1;
    startTimer( 100 );
}


void MediaPlayer::lessReleased() {
    killTimers();
}


void MediaPlayer::moreReleased() {
    killTimers();
}


void MediaPlayer::loopToggled( bool loop ) {
    looping = loop;
}


void MediaPlayer::fullscreenToggled( bool fs ) {
    fullscreen = fs;

    // GO fullscreen...
    if ( fullscreen ) {
	audioUI->reparent( 0, WStyle_Tool | WStyle_Customize | WStyle_StaysOnTop, QPoint(0,0), FALSE );
        audioUI->resize( qApp->desktop()->width(), qApp->desktop()->height() );
	videoUI->reparent( 0, WStyle_Tool | WStyle_Customize | WStyle_StaysOnTop, QPoint(0,0), FALSE );
        videoUI->resize( qApp->desktop()->width(), qApp->desktop()->height() );
    } else {
	// GO normal...
	audioUI->reparent( stack, 0, QPoint(0, 0), FALSE );
        audioUI->resize( videoUI->sizeHint() );
	videoUI->reparent( stack, 0, QPoint(0, 0), FALSE );
        videoUI->resize( videoUI->sizeHint() );
    }

}


void MediaPlayer::holdToggled( bool hold ) {
    paused = hold;
    videoUI->setPaused( hold );
}


void MediaPlayer::timerEvent( QTimerEvent * ) {
    if ( volumeDirection == +1 )
	AudioDevice::increaseVolume();
    else if ( volumeDirection == -1 )
        AudioDevice::decreaseVolume();
}

