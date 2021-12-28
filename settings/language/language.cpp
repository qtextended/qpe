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


LanguageSettings::LanguageSettings( QWidget* parent,  const char* name, WFlags fl )
    : LanguageSettingsBase( parent, name, TRUE, fl )
{
    if ( FontManager::hasUnicodeFont() )
       languages->setFont(FontManager::unicodeFont(FontManager::Proportional));

    QString tfn = QPEApplication::qpeDir()+"/i18n/";
    QDir langDir = tfn;
    QStringList list = langDir.entryList("*", QDir::Dirs );
  
    QStringList::Iterator it;
  
    for( it = list.begin(); it != list.end(); ++it ) {
	QString name = (*it);
	QFileInfo desktopFile( tfn + "/" + name + "/.directory" );
	if( desktopFile.exists() ) {
	    langAvail.append(name);
	    Config conf( desktopFile.filePath(), Config::File );
	    QString langName = conf.readEntry( "Name" );
	    QString ownName = conf.readEntryDirect( "Name["+name+"]" );
	    if ( ownName.isEmpty() )
		ownName = conf.readEntryDirect( "Name" );
	    if ( !ownName.isEmpty() && ownName != langName )
		langName = langName + " [" + ownName + "]";
	    languages->insertItem( langName );
	    
	}
    }
  
    reset();
}

LanguageSettings::~LanguageSettings()
{
}

void LanguageSettings::accept()
{
    applyLanguage();
    QDialog::accept();
}

void LanguageSettings::applyLanguage()
{
  QString lang = langAvail.at( languages->currentItem() );
  setLanguage( lang );
}


void LanguageSettings::reject()
{
    reset();
    QDialog::reject();
}

void LanguageSettings::reset()
{
    QString l = getenv("LANG");
    Config config("language");
    l = config.readEntry( "Language", l );
    if(l.isEmpty()) l = "en";
    actualLanguage = l;

    int n = langAvail.find( l );
    languages->setCurrentItem( n );
}

QString LanguageSettings::actualLanguage;

void LanguageSettings::setLanguage(const QString& lang)
{
  if( lang != actualLanguage ) {
    Config config("locale");
    config.setGroup( "Language" );
    config.writeEntry( "Language", lang );
    config.write();
    
#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
    QCopEnvelope e("QPE/System","language(QString)");
    e << lang;
#endif
  }
}


