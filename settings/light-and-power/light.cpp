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
#include "settings.h"
#include <global.h>
#include <fontmanager.h>
#include <config.h>
#include <applnk.h>
#include <qpeapplication.h>

#include <iostream.h>

#include <qlabel.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qtabwidget.h>
#include <qslider.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qdatastream.h>
#include <qmessagebox.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qlistbox.h>
#include <qdir.h>
#if QT_VERSION >= 300
#include <qstylefactory.h>
#endif
#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
#include "qcopenvelope_qws.h"
#endif

LightSettings::LightSettings( QWidget* parent,  const char* name, WFlags fl )
    : LightSettingsBase( parent, name, TRUE, fl )
{
    // Not supported
    auto_brightness->hide();

    Config config( "qpe" );

    config.setGroup( "Screensaver" );
    // default to 3 minutes
    int interval = config.readNumEntry( "Interval", 180000 );
    diminterval->setValue( interval/60000 );
    
    screensaver->setChecked( config.readNumEntry("Dim",1) != 0 );
    initbright = config.readNumEntry("Brightness",128);
    brightness->setValue( 255 - initbright );

    connect(brightness, SIGNAL(valueChanged(int)), this, SLOT(applyBrightness()));
    connect( (QObject*)screensaver, SIGNAL(toggled(bool)),
             (QObject*)diminterval, SLOT(setEnabled(bool)) );
}

LightSettings::~LightSettings()
{
}

static void set_fl(int bright)
{
    QCopEnvelope e("QPE/System", "setBacklight(int)" );
    e << bright;
}

void LightSettings::reject()
{
    set_fl(initbright);

    QDialog::reject();
}

void LightSettings::accept()
{
    applyBrightness();

    int interval = screensaver->isChecked() ? diminterval->value() * 60000 : 0;
    QCopEnvelope e("QPE/System", "setScreenSaverInterval(int)" );
    e << interval;

    Config config( "qpe" );
    config.setGroup( "Screensaver" );
    config.writeEntry( "Dim", (int)screensaver->isChecked() );
    config.writeEntry( "Interval", interval );
    config.writeEntry( "Brightness", 255-brightness->value() );
    config.write();

    QDialog::accept();
}

void LightSettings::applyBrightness()
{
    int bright = 255-brightness->value();
    set_fl(bright);
}


