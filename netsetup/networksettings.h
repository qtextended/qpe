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
**********************************************************************/
#ifndef NETWORKSETTINGS_H
#define NETWORKSETTINGS_H

#include <qmap.h>
#include "networksettingsbase.h"

class QButtonGroup;

class NetworkSettings : public NetworkSettingsBase
{
    Q_OBJECT
public:
    NetworkSettings();

private slots:
    void ipSelect( int id );

private:
    void readConfig();
    bool writeConfig();
    bool parseLine( QString line, QString &key, QString &value );
    virtual void accept();

private:
    QMap<QString,QString> config;
    QButtonGroup *ipGroup;
};

#endif
