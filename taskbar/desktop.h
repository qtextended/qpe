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
#ifndef __DESKTOP_H__
#define __DESKTOP_H__


#include <qwidget.h>
#include <qpeapplication.h>
#include "shutdownimpl.h"

class Background;
class Launcher;
class TaskBar;

class DesktopApplication : public QPEApplication
{
    Q_OBJECT
public:
    DesktopApplication( int& argc, char **argv, Type t );

signals:
    void home();
    void datebook();
    void contacts();
    void menu();
    void launch();
    void email();
    void backlight();
    void power();

protected:
#ifdef Q_WS_QWS
    bool qwsEventFilter( QWSEvent * );
#endif
    void shutdown();
    void restart();

protected slots:
    void shutdown(ShutdownImpl::Type);
};


class Desktop : public QWidget {
    Q_OBJECT
public:
    Desktop();
    ~Desktop();

public slots:
    void raiseDatebook();
    void raiseContacts();
    void raiseMenu();
    void raiseLauncher();
    void raiseEmail();
    void togglePower();
    void toggleLight();

protected:
    void executeOrModify(const QCString& app);
    void styleChange( QStyle & );

    QWidget *bg;
    Launcher *launcher;
    TaskBar *tb;
};


#endif // __DESKTOP_H__

