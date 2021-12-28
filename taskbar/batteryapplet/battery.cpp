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
#include "battery.h"
#include "batterystatus.h"
#include <alarmserver.h>
#include <qpainter.h>
#ifdef QT_QWS_CUSTOM
#include "custom.h"
#endif

#include <unistd.h>
#include <stdio.h>
#ifdef QT_QWS_IPAQ_NO_APM
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/h3600_ts.h>
#endif


BatteryMeter::BatteryMeter( QWidget *parent = 0 ) : QWidget( parent )
{
    FILE *f = fopen("/proc/apm", "r");
    if ( f ) {
	fclose(f);
	haveApm = TRUE;
    } else {
	haveApm = FALSE;
    }

    getStatus( ac_status, 0, 0, percent, seconds );
    startTimer( 5000 );
    setFixedHeight(12);
    show();
}

BatteryMeter::~BatteryMeter()
{
}

QSize BatteryMeter::sizeHint() const
{
    return QSize(6,12);
}

void BatteryMeter::mouseReleaseEvent( QMouseEvent *)
{
    if ( batteryView ) {
        if ( batteryView->isVisible() )
            delete (QWidget *) batteryView;
        else
            batteryView->raise();
        return;
    }

    // Create a small window to view the battery status
    batteryView = new BatteryStatus( this );
}

void BatteryMeter::timerEvent( QTimerEvent * )
{
    int a, p, s;
    getStatus( a, 0, 0, p, s );
    if ( ac_status != a || percent != p || seconds != s ) {
	ac_status = a;
	percent = p;
	seconds = s;
	repaint(FALSE);
    }
}

void BatteryMeter::getStatus( int &ac_status, int *bat_status, int *bat_flag, int &percent, int &seconds )
{
    bool usedApm = FALSE;

    // Some iPAQ kernel builds don't have APM. If this is not the case we
    // save ourselves an ioctl by testing if /proc/apm exists in the
    // constructor and we use /proc/apm instead
    int bs=0, bf=0;
    if (haveApm) { // Standard APM
        FILE *f = fopen("/proc/apm", "r");
	if ( f ) {
	    //I 1.13 1.2 0x02 0x00 0xff 0xff 49% 147 sec
	    char u;
	    fscanf(f, "%*[^ ] %*d.%*d 0x%*x 0x%x 0x%x 0x%x %d%% %i %c",
		&ac_status, &bs, &bf, &percent, &seconds, &u);
	    fclose(f);
	    switch ( u ) {
		case 'm': seconds *= 60;
		case 's': break; // ok
		default: seconds = -1; // unknown
	    }
	    usedApm = TRUE;
	}
    }

    if (!usedApm) {
#ifdef QT_QWS_IPAQ_NO_APM
	int fd;
	int err;
	struct bat_dev batt_info;

	memset(&batt_info, 0, sizeof(batt_info));

	fd = ::open("/dev/ts",O_RDONLY);
	if( fd < 0 )
	    return;

	ioctl(fd, GET_BATTERY_STATUS, &batt_info);
	ac_status = batt_info.ac_status;
	percent = ( 425 * batt_info.batt1_voltage ) / 1000 - 298; // from h3600_ts.c
	seconds = 0;
	::close (fd);
#else
	ac_status = 0;
	percent = 100;
	seconds = 3600;
	return;
#endif
    }
    if ( percent > 100 )
	percent = -1;
#ifdef QPE_FAKEPERCENTAGE
    // percentage is bogus on sharp
    percent = -1;
#endif

    if ( !bat_status && !bat_flag ) {
	if ( percent < 0 ) {
	    // fake percentage
	    switch ( bs ) {
		case 0x00: percent = 100; break; // High
		case 0x01: percent = 30; break; // Low
		case 0x7f: percent = 10; break; // Very Low
		case 0x02: percent = 5; break; // Critical
		case 0x03: percent = 50; break; // Charging
	    }
	}
    } else {
	if ( bat_status ) *bat_status = bs;
	if ( bat_flag ) *bat_flag = bf;
    }
}

void BatteryMeter::paintEvent( QPaintEvent* )
{
    QPainter p(this);

    if (  seconds < 600 && percent < 10 && !ac_status ) {
        // we should warn the user if there are less than 10 minutes left.
	// -- but this is not the way --WWA
        //AlarmServer::addAlarm( AlarmServer::MessageBox, QDateTime::currentDateTime(), "Battery is running low." );
    }


    QColor c;
    QColor darkc;
    QColor lightc;
    if ( ac_status == 0x00 ) {
	c = blue.light(120);
	darkc = c.dark(120);
	lightc = c.light(140);
    } else if ( ac_status == 0x01 ) {
	c = green.dark(130);
	darkc = c.dark(120);
	lightc = c.light(180);
    } else {
	c = red;
	darkc = c.dark(120);
	lightc = c.light(160);
    }

    int w = 6;
    int h = height()-3;
    int pix = (percent * h) / 100;
    int y2 = height() - 2;
    int y = y2 - pix;
    int x1 = (width() - w) / 2;

    p.setPen(QColor(80,80,80));
    p.drawLine(x1+w/4,0,x1+w/4+w/2,0);
    p.drawRect(x1,1,w,height()-1);
    p.setBrush(c);

    int extra = ((percent * h) % 100)/(100/4);

#define Y(i) ((i<=extra)?y-1:y)
#define DRAWUPPER(i) if ( Y(i) >= 2 ) p.drawLine(i+x1,2,i+x1,Y(i));
    p.setPen(  gray );
    DRAWUPPER(1);
    DRAWUPPER(3);
    p.setPen( gray.light(130) );
    DRAWUPPER(2);
    p.setPen( gray.dark(120) );
    DRAWUPPER(4);

#define DRAW(i) { if ( Y(i) < y2 ) p.drawLine(i+x1,Y(i)+1,i+x1,y2); }
    p.setPen( c );
    DRAW(1);
    DRAW(3);
    p.setPen( lightc );
    DRAW(2);
    p.setPen(darkc);
    DRAW(4);
}

