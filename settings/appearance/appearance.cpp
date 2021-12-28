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
#if defined(QT_QWS_IPAQ) || defined(QT_QWS_EBX)
#include <unistd.h>
#include <linux/fb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#endif
#include <stdlib.h>


AppearanceSettings::AppearanceSettings( QWidget* parent,  const char* name, WFlags fl )
    : AppearanceSettingsBase( parent, name, TRUE, fl )
{
#if QT_VERSION >= 300
    styleList->insertStringList(QStyleFactory::styles());
#else
    styleList->insertItem( "Windows" );
    styleList->insertItem( "Light" );
#ifndef QT_NO_STYLE_MOTIF
    styleList->insertItem( "Motif" );
#endif
#ifndef QT_NO_STYLE_MOTIFPLUS
    styleList->insertItem( "MotifPlus" );
#endif
#ifndef QT_NO_STYLE_PLATINUM
    styleList->insertItem( "Platinum" );
#endif
#endif
    styleList->insertItem( "QPE" );

    populateColorList();

    Config config("qpe");
    config.setGroup( "Appearance" );
    QString s = config.readEntry( "Style", "Light" );
    styleList->setCurrentItem( styleList->findItem( s ) );

    s = config.readEntry( "Scheme", "Desert" );
    colorList->setCurrentItem( colorList->findItem( s ) );
}

AppearanceSettings::~AppearanceSettings()
{
}

void AppearanceSettings::accept()
{
    Config config("qpe");
    config.setGroup( "Appearance" );

    QString s = styleList->currentText();
    config.writeEntry( "Style", s );

    s = colorList->currentText();
    config.writeEntry( "Scheme", s );

    Config scheme( QPEApplication::qpeDir() + "etc/colors/" + s + ".scheme",
        Config::File );
    QString color = scheme.readEntry( "Background", "#E5E1D5" );
    config.writeEntry( "Background", color );
    color = scheme.readEntry( "Button", "#D6CDBB" );
    config.writeEntry( "Button", color );
    color = scheme.readEntry( "Highlight", "#800000" );
    config.writeEntry( "Highlight", color );
    color = scheme.readEntry( "HighlightedText", "#FFFFFF" );
    config.writeEntry( "HighlightedText", color );
    color = scheme.readEntry( "Text", "#000000" );
    config.writeEntry( "Text", color );
    color = scheme.readEntry( "ButtonText", "#000000" );
    config.writeEntry( "ButtonText", color );
    color = scheme.readEntry( "Base", "#FFFFFF" );
    config.writeEntry( "Base", color );

    config.write(); // need to flush the config info first
    Global::applyStyle();
    QDialog::accept();
}

void AppearanceSettings::populateColorList()
{
    QDir dir( QPEApplication::qpeDir() + "etc/colors" );
    QStringList list = dir.entryList( "*.scheme" );
    for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
        QString name = (*it).left( (*it).find( ".scheme" ) );
        colorList->insertItem( name );
    }
}


