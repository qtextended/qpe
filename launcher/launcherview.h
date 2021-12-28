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
#ifndef LAUNCHERVIEW_H
#define LAUNCHERVIEW_H

#include <qvbox.h>

class AppLnk;
class AppLnkSet;
class LauncherIconView;
class QIconView;
class QIconViewItem;
class MenuButton;

class LauncherView : public QVBox
{ 
    Q_OBJECT

public:
    LauncherView( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~LauncherView();

    bool removeLink(const QString& linkfile);
    void addItem(AppLnk* app, bool resort=TRUE);
    void sort();

    void setToolsEnabled(bool);
    void updateTools();

    void setBusy(bool);

public slots:
    void populate( AppLnkSet *folder, const QString& categoryfilter );

signals:
    void clicked( const AppLnk * );
    void rightPressed( AppLnk * );

protected slots:
    void selectionChanged();
    void returnPressed( QIconViewItem *item );
    void itemClicked( int, QIconViewItem * );
    void itemPressed( int, QIconViewItem * );
    void sortBy(int);
    void showType(const QString&);
    void showCategory(const QString&);

protected:
    void internalPopulate( AppLnkSet *, const QString& categoryfilter );

private:
    static bool bsy;
    QWidget* tools;
    LauncherIconView* icons;
    MenuButton *sortmb, *typemb, *catmb;
};

#endif // LAUNCHERVIEW_H
