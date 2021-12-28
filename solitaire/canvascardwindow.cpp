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
#include <qmainwindow.h>
#include <qpopupmenu.h>
#include <resource.h>
#include "canvascardwindow.h"
#include "patiencecardgame.h"
#include "freecellcardgame.h"


CanvasCardWindow::CanvasCardWindow(QWidget* parent, const char* name, WFlags f) :
    QMainWindow(parent, name, f), canvas(230, 260), snapOn(TRUE), cardBack(4), gameType(0),
    cardGame(NULL)
{
    setIcon( Resource::loadPixmap( "cards" ) );

    // Create Playing Area for Games
    if ( QPixmap::defaultDepth() < 12 ) {
//      canvas.setBackgroundColor(QColor(0x51, 0x74, 0x6B));
//	canvas.setBackgroundColor(QColor(0x20, 0xb0, 0x50));
	canvas.setBackgroundColor(QColor(0x08, 0x98, 0x2D));
    } else {
        QPixmap bg;
	bg.convertFromImage( Resource::loadImage( "table_pattern" ), ThresholdDither );  
        canvas.setBackgroundPixmap(bg);
    }

#if defined( QT_QWS_CASSIOPEIA )
    canvas.setAdvancePeriod(70);
#else
    canvas.setAdvancePeriod(30);
#endif

#ifdef _PATIENCE_USE_ACCELS_
    QMenuBar* menu = menuBar();

    QPopupMenu* file = new QPopupMenu;
    file->insertItem(tr("Patience"), this, SLOT(initPatience()), CTRL+Key_F);
    file->insertItem(tr("Freecell"), this, SLOT(initFreecell()), CTRL+Key_F);
    menu->insertItem(tr("&Game"), file);
    
    menu->insertSeparator();

    QPopupMenu* settings = new QPopupMenu;
    settings->insertItem(tr("&Change Card Backs"), this, SLOT(changeCardBacks()), Key_F2);
    settings->insertItem(tr("&Snap On/Off"), this, SLOT(snapToggle()), Key_F3);
    menu->insertItem(tr("&Settings"),settings);

    menu->insertSeparator();

    QPopupMenu* help = new QPopupMenu;
    help->insertItem(tr("&About"), this, SLOT(help()), Key_F1);
    help->setItemChecked(dbf_id, TRUE);
    menu->insertItem(tr("&Help"),help);
#else
    QMenuBar* menu = menuBar();

    QPopupMenu* file = new QPopupMenu;
    file->insertItem(tr("Patience"), this, SLOT(initPatience()));
    file->insertItem(tr("Freecell"), this, SLOT(initFreecell()));
    menu->insertItem(tr("New Game"), file);
    
    menu->insertSeparator();

    QPopupMenu* settings = new QPopupMenu;
    settings->insertItem(tr("Change Card Backs"), this, SLOT(changeCardBacks()));
    settings->insertItem(tr("Snap On/Off"), this, SLOT(snapToggle()));
    settings->insertItem(tr("Change Cards Drawn"), this, SLOT(drawnToggle()));
    menu->insertItem(tr("Settings"),settings);

#endif

    menu->show();


    Config cfg( "Patience" );
    cfg.setGroup( "GlobalSettings" );
    gameType = cfg.readNumEntry( "GameType", -1 );
    if ( gameType == 0 ) {
	cardGame = new PatienceCardGame( &canvas, snapOn, this );
	setCaption(tr("Patience"));
	setCentralWidget(cardGame);
	cardGame->readConfig( cfg );
	setCardBacks();
    } else if ( gameType == 1 ) {
	cardGame = new FreecellCardGame( &canvas, snapOn, this );
	setCaption(tr("Freecell"));
	setCentralWidget(cardGame);
	//cardGame->newGame(); // Until we know how to handle reading freecell config
	cardGame->readConfig( cfg );
	setCardBacks();
    } else {
	// Probably there isn't a config file or it is broken
	// Start a new game
	initPatience();
    }
}


CanvasCardWindow::~CanvasCardWindow()
{
    if (cardGame) {
	Config cfg("Patience");
	cfg.setGroup( "GlobalSettings" );
	cfg.writeEntry( "GameType", gameType );
	cardGame->writeConfig( cfg );
	delete cardGame;
    }
}


void CanvasCardWindow::resizeEvent(QResizeEvent *)
{
    QSize s = centralWidget()->size();
    int fw = style().defaultFrameWidth();
    canvas.resize( s.width() - fw - 2, s.height() - fw - 2);
}


void CanvasCardWindow::initPatience()
{
    // Create New Game 
    if ( cardGame )
	delete cardGame;
    cardGame = new PatienceCardGame( &canvas, snapOn, this );
    gameType = 0;
    setCaption(tr("Patience"));
    setCentralWidget(cardGame);
    cardGame->newGame();
    setCardBacks();
}


void CanvasCardWindow::initFreecell()
{
    // Create New Game
    if ( cardGame ) {
	delete cardGame;
    }
    cardGame = new FreecellCardGame( &canvas, snapOn, this );
    gameType = 1;
    setCaption(tr("Freecell"));
    setCentralWidget(cardGame);
    cardGame->newGame();
    setCardBacks();
}


void CanvasCardWindow::snapToggle()
{
    snapOn = !snapOn;
    cardGame->toggleSnap();
}


void CanvasCardWindow::drawnToggle()
{
    cardGame->toggleCardsDrawn();
}


void CanvasCardWindow::setCardBacks()
{
    QCanvasItemList l = canvas.allItems();

    for (QCanvasItemList::Iterator it = l.begin(); it != l.end(); ++it) {
	if ( (*it)->rtti() == canvasCardId )
	    ((CanvasCard *)(*it))->setCardBack( cardBack );
    }
}


void CanvasCardWindow::changeCardBacks()
{
    cardBack++;

    if (cardBack == 5)
	cardBack = 0;

    setCardBacks();
}


