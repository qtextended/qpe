/**********************************************************************
** Copyright (C) 2001 Trolltech AS.  All rights reserved.
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
** $Id: citytime.cpp,v 1.18 2001/08/07 07:15:51 warwick Exp $
**
**********************************************************************/

#include <cstdlib>

#include <qdir.h>
#include <qfile.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qpeapplication.h>
#include <qtextstream.h>
#include <qtoolbutton.h>
#include <qregexp.h>
#include <config.h>
#include <tzselect.h>
#include <timestring.h>

#include "zonemap.h"
#include "citytime.h"
#include "config.h"


CityTime::CityTime( QWidget *parent, const char* name,
                            WFlags fl )
    : CityTimeBase( parent, name, fl ),
      strRealTz(0),
      bAdded(false)
{
    Config config( "qpe" );
    bWhichClock = config.readBoolEntry( "AMPM" );
    frmMap->changeClock( bWhichClock );

    char *pEnv;
    pEnv = NULL;
    pEnv = getenv("TZ");
    if ( pEnv )
        strRealTz = pEnv;
    pEnv = NULL;
    pEnv = getenv("HOME");
    if ( pEnv )
        strHome = pEnv;
    // append the labels to their respective lists...
    listCities.setAutoDelete( true );
    listTimes.setAutoDelete( true );
    
    listCities.append( cmdCity1 );
    listCities.append( cmdCity2 );
    listCities.append( cmdCity3 );

    listTimes.append( lblCTime1 );
    listTimes.append( lblCTime2 );
    listTimes.append( lblCTime3 );


    // kludgy way of getting the screen size so we don't have to depend
    // on a resize event...
    QWidget *d = QApplication::desktop();
    if ( d->width() < d->height() ) {
        // append for that 4 down look

        listCities.append( cmdCity4 );
        listCities.append( cmdCity5 );
        listCities.append( cmdCity6 );
        listTimes.append( lblCTime4 );
        listTimes.append( lblCTime5 );
        listTimes.append( lblCTime6 );
        lblCTime7->hide();
        lblCTime8->hide();
        lblCTime9->hide();
        cmdCity7->hide();
        cmdCity8->hide();
        cmdCity9->hide();
    } else {
        listCities.append( cmdCity7 );
        listCities.append( cmdCity8 );
        listCities.append( cmdCity9 );
        listTimes.append( lblCTime7 );
        listTimes.append( lblCTime8 );
        listTimes.append( lblCTime9 );
        lblCTime4->hide();
        lblCTime5->hide();
        lblCTime6->hide();
        cmdCity4->hide();
        cmdCity5->hide();
        cmdCity6->hide();
    }
    bAdded = true;
    readInTimes();
    QObject::connect( qApp, SIGNAL( clockChanged(bool) ),
                      this, SLOT( changeClock(bool) ) );
    // now start the timer so we can update the time quickly every second
    showTime();
    startTimer( 1000 );
}

CityTime::~CityTime()
{
    Config cfg("CityTime");
    cfg.setGroup("TimeZones");
    QListIterator<QToolButton> itCity( listCities );
    int i;
    for ( i = 0, itCity.toFirst();  i < CITIES; i++, ++itCity ) {
	if ( !strCityTz[i].isNull() ) {
	    cfg.writeEntry("Zone"+QString::number(i), strCityTz[i]);
	    cfg.writeEntry("ZoneName"+QString::number(i), itCity.current()->text());
	}
    }

    // restore the timezone, just in case we messed with it and
    // are destroyed at an inoppurtune moment
    if ( !strRealTz.isNull() ) {
        // this should be checked, but there is not much that can done at this
        //point if it fails
        setenv( "TZ", strRealTz, true );
    }
}

void CityTime::timerEvent( QTimerEvent * )
{
    // change the time again!!
    showTime();
}

void CityTime::mousePressEvent( QMouseEvent * )
{
    // DEBUG enable this to get a look at the zone information DEBUG
//    frmMap->showZones();
}

void CityTime::showTime( void )
{
    int i;
    QListIterator<QLabel> itTime(listTimes);

    // traverse the list...
    for ( i = 0, itTime.toFirst(); i < CITIES; i++, ++itTime) {
        if ( !strCityTz[i].isNull() ) {
            if ( setenv( "TZ", strCityTz[i], true ) == 0 ) {
                itTime.current()->setText( shortTime(bWhichClock) );
            } else {
                QMessageBox::critical( this, tr( "Time Changing" ),
                tr( "There was a problem setting timezone %1" )
                .arg( QString::number( i + 1 ) ) );
            }
        }
    }
    // done playing around... put it all back
    unsetenv( "TZ" );
    if ( !strRealTz.isNull() ) {
        if ( setenv( "TZ", strRealTz, true ) != 0 ) {
            QMessageBox::critical( this, tr( "Restore Time Zone" ),
            tr( "There was a problem setting your timezone."
            "Your time may be wrong now..." ) );
        }
    }
}

void CityTime::slotNewTz( const QString & strNewCountry,
                              const QString & strNewCity )
{
    // determine what to do based on what putton is pressed...
    QListIterator<QToolButton> itCity(listCities);
    int i;
    // go through the list and make adjustments based on which button is on
    for ( i = 0, itCity.toFirst(); itCity.current(), i < CITIES; i++, ++itCity ) {
        QToolButton *cmdTmp = itCity.current();
        if ( cmdTmp->isOn() ) {
            strCityTz[i] = strNewCountry + strNewCity;
            QString s = strNewCity;
            cmdTmp->setText( s.replace( QRegExp("_"), " " ) );
            cmdTmp->toggle();
            // we can actually break, since there is only one button
            // that is ever pressed!
            break;
        }
    }
    showTime();
}

void CityTime::readInTimes( void )
{
    Config cfg("CityTime");
    cfg.setGroup("TimeZones");
    QListIterator<QToolButton> itCity( listCities );
    int i=0;
    for ( ; i < CITIES ; i++ ) {
	QString zn = cfg.readEntry("Zone"+QString::number(i), QString::null);
	if ( zn.isNull() )
	    break;
	QString nm = cfg.readEntry("ZoneName"+QString::number(i));
	strCityTz[i] = zn;
	itCity.current()->setText(nm);
	++itCity;
    }
    if ( !i ) {
        // write in our own in a shameless self promotion and some humor
        QStringList list = timezoneDefaults();
        int i;
        QStringList::Iterator it = list.begin();
        for ( i = 0, itCity.toFirst(); i < CITIES && itCity.current();
              i++, ++itCity ) {
            strCityTz[i] = *it++;
            itCity.current()->setText( *it++ );
        }
    }
}

void CityTime::changeClock( bool newClock )
{
    bWhichClock = newClock;
    showTime();
}
