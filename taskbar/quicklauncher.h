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
#ifndef __QUICK_LAUNCHER_H__
#define __QUICK_LAUNCHER_H__


#include <qhbox.h>

class AppLnk;


class QuickLauncher : public QHBox {
    Q_OBJECT
public:
    QuickLauncher( QWidget *parent );
    void addLauncherButton( AppLnk * );
};


#endif // __QUICK_LAUNCHER_H__


