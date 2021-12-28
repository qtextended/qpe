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
#include <qmessagebox.h>
#include <qsound.h>
#include <qfile.h>
#include <qdir.h>
#include "qpeapplication.h"
#include "resource.h"
#include "global.h"
#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
#include "qcopenvelope_qws.h"
#endif
#include "alarmserver.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

struct timerEventItem {
    QDateTime	when;
    QCString channel, message;
    int data;
};

class TimerRecieverObject : public QObject
{
public:
  TimerRecieverObject() { }
  ~TimerRecieverObject() { }
  void resetTimer();
  void setTimerEventItem();
protected:
  void timerEvent( QTimerEvent *te );
};

TimerRecieverObject *timerEventReceiver = NULL;
QList<timerEventItem> *timerEventList = NULL;
timerEventItem *nearestTimerEvent = NULL;

// set the timer to go off on the next event in the list
void setNearestTimerEvent()
{
    nearestTimerEvent = NULL;
    if ( timerEventList ) {
	QListIterator<timerEventItem> it( *timerEventList );
	if ( *it )
	    nearestTimerEvent = *it;
	for ( ; *it; ++it )
	    if ( (*it)->when < nearestTimerEvent->when )
	        nearestTimerEvent = *it;
        if (nearestTimerEvent)
            timerEventReceiver->resetTimer();
    }
}

void TimerRecieverObject::resetTimer()
{
    const int maxsecs = 2147000;
    QDateTime nearest = nearestTimerEvent->when;
    QDateTime now = QDateTime::currentDateTime();
    int secs = now.secsTo(nearest);
    if ( secs > maxsecs ) {
	// too far for millisecond timing
	nearest = QDateTime::currentDateTime().addSecs(maxsecs);
	secs = maxsecs;
    }

    // System timer (needed so that we wake from deep sleep),
    // from the Epoch in seconds.
    //
    QDateTime epoch; epoch.setTime_t(0);
    int at_secs = epoch.secsTo(nearest);
    // qDebug("reset timer to %d seconds from Epoch",at_secs);
    QString atdir = "/var/spool/at/";
    QString fn = atdir + QString::number(at_secs) + "."
		+ QString::number(getpid());
    QFile atfile(fn+".new");
    if ( atfile.open(IO_WriteOnly) ) {
	// just wake up and delete the at file
	QString cmd = "#!/bin/sh\nrm " + fn;
	atfile.writeBlock(cmd.latin1(),cmd.length());
	atfile.close();
	QDir d; d.rename(fn+".new",fn);
	chmod(fn.latin1(),0755);
	QFile trigger(atdir+"trigger");
	trigger.open(IO_WriteOnly);
	trigger.writeBlock("\n",1);
    } else {
	qWarning("Cannot open atd file %s",fn.latin1());
    }

    // Qt timers (does the actual alarm)
    // from now in milliseconds
    //
    qDebug("AlarmServer waiting %d seconds",secs);
    startTimer( 1000 * secs + 500 );
}

void TimerRecieverObject::timerEvent( QTimerEvent *te )
{
    killTimers();
    if (nearestTimerEvent) {
        if (nearestTimerEvent->when <= QDateTime::currentDateTime()) {
	    QCopEnvelope e(nearestTimerEvent->channel,
			    nearestTimerEvent->message);
	    e << nearestTimerEvent->when << nearestTimerEvent->data;
	    if ( timerEventList )
		timerEventList->remove( nearestTimerEvent );
	}
        setNearestTimerEvent();
    } else {
        resetTimer();
    }
}

/*!
  \class AlarmServer alarmserver.h
  \brief The AlarmServer class provides alarms to be scheduled.

  Applications which wish to be informed when a certain time instant
  passes use the functions of AlarmServer to request so.
*/

/*!
  \internal
*/
AlarmServer::AlarmServer()
{
}

/*!
  Schedules an alarm for \a when. Soon after this time,
  \a message will be sent to \a channel, with \a data as
  a parameter. \a message must be of the form "someMessage(int)".

  \sa deleteAlarm()
*/
void AlarmServer::addAlarm ( QDateTime when, const QCString& channel, const QCString& message, int data)
{
    if ( qApp->type() == QApplication::GuiServer ) {
	// Here we are the server so either it has been directly called from
	// within the server or it has been sent to us from a client via QCop
	if (!timerEventList) {
	    timerEventList = new QList<timerEventItem>;
	}
	if (!timerEventReceiver)
	    timerEventReceiver = new TimerRecieverObject;
      
	timerEventItem *newTimerEventItem = new timerEventItem;
	newTimerEventItem->when = when;
	newTimerEventItem->channel = channel;
	newTimerEventItem->message = message;
	newTimerEventItem->data = data;
	timerEventList->append( newTimerEventItem );

	// quicker than using setNearestTimerEvent()
	if ( nearestTimerEvent ) {
	    if (newTimerEventItem->when < nearestTimerEvent->when) {
	        nearestTimerEvent = newTimerEventItem;
	        timerEventReceiver->killTimers();
	        timerEventReceiver->resetTimer();
	    }
	} else {
	    nearestTimerEvent = newTimerEventItem;
	    timerEventReceiver->resetTimer();
	}
    } else {
        QCopEnvelope e( "QPE/System", "addAlarm(QDateTime,QCString,QCString,int)" );
        e << when << channel << message << data;
    }
}

/*!
  Deletes previously scheduled alarms which match \a when, \a channel, \a message,
  and \a data.

  Passing null values for \a when, \a channel, or \a message indicates "any".
  Passing -1 for \a data indicates "any".

  \sa deleteAlarm()
*/
void AlarmServer::deleteAlarm (QDateTime when, const QCString& channel, const QCString& message, int data)
{
    if ( qApp->type() == QApplication::GuiServer) {
	if ( timerEventList != NULL && timerEventReceiver != NULL ) {
	    timerEventReceiver->killTimers();

	    // iterate over the list of events
	    QListIterator<timerEventItem> it( *timerEventList );
	    for ( ; *it; ++it ) {
		// if its a match, delete it
		if ( ( (*it)->when == when || when.isNull() ) 
		    && ( channel.isNull() || (*it)->channel == channel )
		    && ( message.isNull() || (*it)->message == message )
		    && ( data==-1 || (*it)->data == data ) )
		{
		    // if it's first, then we need to update the timer
		    if ( (*it) == nearestTimerEvent ) {
			timerEventList->remove(*it);
			setNearestTimerEvent();
		    } else {
			timerEventList->remove(*it);
		    }
		}
	    }
	    if ( nearestTimerEvent )
		timerEventReceiver->resetTimer();
	}
    } else {
        QCopEnvelope e( "QPE/System", "deleteAlarm(QDateTime,QCString,QCString,int)" );
        e << when << channel << message << data;
    }
}
