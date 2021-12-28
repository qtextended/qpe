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
#include <stdlib.h>
#include "qpeapplication.h"
#include "config.h"
#include "applnk.h"
#include "global.h"
#include "resource.h"
#include "sidething.h"
#include "mrulist.h"
#include "info.h"
#include "startmenu.h"
#include <qdict.h>


// #define USE_CONFIG_FILE


StartMenu::StartMenu(QWidget *parent) : QLabel( parent )
{
    loadOptions();

    setPixmap( Resource::loadPixmap( startButtonPixmap ) );
    setFocusPolicy( NoFocus );
    //setFlat( startButtonIsFlat );

    apps = new AppLnkSet( QPEApplication::qpeDir() + "apps" );

    createMenu();
}


void StartMenu::mousePressEvent( QMouseEvent * )
{
    launch();
    if (desktopInfo)
        desktopInfo->menuClicked();
}


StartMenu::~StartMenu()
{
    delete apps;
}


void StartMenu::loadOptions()
{
#ifdef USE_CONFIG_FILE
    // Read configuration file
    Config config("StartMenu");
    config.setGroup( "StartMenu" );
    QString tmpBoolString1 = config.readEntry( "UseWidePopupMenu", "FALSE" );
    useWidePopupMenu  = ( tmpBoolString1 == "TRUE" ) ? TRUE : FALSE;
    QString tmpBoolString2 = config.readEntry( "StartButtonIsFlat", "TRUE" );
    startButtonIsFlat = ( tmpBoolString2 == "TRUE" ) ? TRUE : FALSE;
    QString tmpBoolString3 = config.readEntry( "UseMRUList", "TRUE" );
    useMRUList        = ( tmpBoolString3 == "TRUE" ) ? TRUE : FALSE;
    popupMenuSidePixmap = config.readEntry( "PopupMenuSidePixmap", "sidebar" );
    startButtonPixmap = config.readEntry( "StartButtonPixmap", "go" );
#else
    // Basically just #include the .qpe_menu.conf file settings
    useWidePopupMenu = FALSE;
    popupMenuSidePixmap = "sidebar";
    startButtonIsFlat = TRUE;
    startButtonPixmap = "go";
    useMRUList = TRUE;
#endif
}


void StartMenu::createMenu()
{
    if ( useWidePopupMenu )
	launchMenu = new PopupWithLaunchSideThing( this, &popupMenuSidePixmap );
    else
        launchMenu = new QPopupMenu( this );

    loadMenu( apps, launchMenu );

    if ( useMRUList ) {
	launchMenu->insertSeparator();
	launchMenu->insertItem( new MRUList( launchMenu ) );
    }
}

void StartMenu::itemSelected( int id )
{
    const AppLnk *app = apps->find( id );
    if ( app ) {
	MRUList::addTask( app );
	app->execute();
    }
}

bool StartMenu::loadMenu( AppLnkSet *folder, QPopupMenu *menu )
{
    bool result = FALSE;

    QStringList typs = folder->types();
    QDict<QPopupMenu> typpop;
    for (QStringList::Iterator tit=typs.begin(); tit!=typs.end(); ++tit) {
	if ( !(*tit).isEmpty() ) {
	    QPopupMenu *new_menu = new QPopupMenu( menu );
	    typpop.insert(*tit, new_menu);
	    connect( new_menu, SIGNAL(activated(int)), SLOT(itemSelected(int)) );
	    menu->insertItem( folder->typePixmap(*tit), *tit, new_menu );
	}
    }

    QListIterator<AppLnk> it( folder->children() );
    for ( ; it.current(); ++it ) {
	AppLnk *app = it.current();
	if ( app->type() == "Separator" ) {
	    menu->insertSeparator();
	} else {
	    QString t = app->type();
	    QPopupMenu* pmenu = typpop.find(t);
	    if ( !pmenu )
		pmenu = menu;
	    pmenu->insertItem( app->pixmap(), app->name(), app->id() );
	    result=TRUE;
	}
    }

    if ( result )
	connect( menu, SIGNAL(activated(int)), SLOT(itemSelected(int)) );

    return result;
}


void StartMenu::launch()
{
    int y = mapToGlobal( QPoint() ).y() - launchMenu->sizeHint().height();
    launchMenu->popup( QPoint( 1, y ) );
}


