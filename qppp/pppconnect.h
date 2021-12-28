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
#ifndef PPPCONNECT_H
#define PPPCONNECT_H
#include "pppconnectbase.h"
#include "config.h"
#include <qmap.h>

class PppConnect : public PppConnectBase {
    Q_OBJECT
public:
    PppConnect();
    void done(int);

private slots:
    void goGo();

private:
    void readConfig();
    void writeConfig();
    void findPppPid();
    void init();
    Config config;
    int oldpid;
    QString makeIp(const QString& pattern);
    QMap<QString,QString> pw;
};

#endif
