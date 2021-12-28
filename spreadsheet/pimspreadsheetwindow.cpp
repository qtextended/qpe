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
#include "pimspreadsheetwindow.h"
#include "fileselector.h"
#include "spreadsheet.h"
#include "global.h"
#include "resource.h"
#include <qmenubar.h>
#include <qtoolbar.h>
#include <qlineedit.h>
#include <qaction.h>
#include <qapplication.h>
#include <qtoolbutton.h>
#include <qfileinfo.h>


PIMSpreadsheetWindow::PIMSpreadsheetWindow ( QWidget * parent, const char * name, WFlags f )
    : QMainWindow(parent,name,f)
{
    setCaption( tr("Spreadsheet") );
    setIcon( Resource::loadPixmap( "spreadsheet_icon" ) );

    spreadsheet = new SpreadsheetWidget( this );
    spreadsheet->spreadsheet->setEditor( spreadsheet->equationEdit );

    setToolBarsMovable( FALSE );
    QToolBar *bar = new QToolBar( this );
    bar->setHorizontalStretchable( TRUE );
    menu = bar;

    QMenuBar *mb = new QMenuBar( bar );

    QPopupMenu *file = new QPopupMenu( this );
    QPopupMenu *edit = new QPopupMenu( this );

    //bar = new QToolBar( this );
    editBar = bar;

    QAction *a = new QAction( tr( "New" ), Resource::loadPixmap( "new" ), QString::null, 0, this, 0 );
    connect( a, SIGNAL( activated() ), this, SLOT( fileNew() ) );
    a->addTo( bar );
    a->addTo( file );

    a = new QAction( tr( "Open" ), Resource::loadPixmap( "fileopen" ), QString::null, 0, this, 0 );
    connect( a, SIGNAL( activated() ), this, SLOT( fileOpen() ) );
    a->addTo( bar );
    a->addTo( file );
    
    a = new QAction( tr( "Close" ), Resource::loadPixmap( "close" ), QString::null, 0, this, 0 );
    connect( a, SIGNAL( activated() ), this, SLOT( fileClose() ) );
    a->addTo( file );

    a = new QAction( tr( "Equation" ), Resource::loadPixmap( "calculator" ), QString::null, 0, this, 0 );
    connect( a, SIGNAL( activated() ), spreadsheet->spreadsheet, SLOT( editEquation() ) );
    a->addTo( bar );
    a->addTo( edit );

    a = new QAction( tr( "Clear Cell" ), Resource::loadPixmap( "cut" ), QString::null, 0, this, 0 );
    connect( a, SIGNAL( activated() ), spreadsheet->spreadsheet, SLOT( clearCurrent() ) );
    a->addTo( bar );
    a->addTo( edit );

    mb->insertItem( tr( "File" ), file );
    mb->insertItem( tr( "Edit" ), edit );

    fileSelector = new FileSelector( "application/x-qpe-ss", this, "fileselector" );
    fileSelector->setGeometry(0,0,width(),height());
    connect( fileSelector, SIGNAL( closeMe() ), this, SLOT( closeFileSelector() ) );
    connect( fileSelector, SIGNAL( newSelected( const DocLnk &) ), this, SLOT( newFile( const DocLnk & ) ) );
    connect( fileSelector, SIGNAL( fileSelected( const DocLnk &) ), this, SLOT( openFile( const DocLnk & ) ) );
    fileSelector->setNewVisible(FALSE);
    fileSelector->setCloseVisible( FALSE );
    setCentralWidget( spreadsheet );
    setCentralWidget( fileSelector );
    if (fileSelector->fileCount())
        fileOpen();
    else
	fileNew();
    resize( 200, 300 );
}

PIMSpreadsheetWindow::~PIMSpreadsheetWindow()
{
    spreadsheet->spreadsheet->save();
}

void PIMSpreadsheetWindow::fileNew()
{
    fileSelector->reread();
    closeFileSelector();
    spreadsheet->spreadsheet->save();
    spreadsheet->spreadsheet->viewport()->setFocus();
    DocLnk f;
    spreadsheet->spreadsheet->newFile( f );
}

void PIMSpreadsheetWindow::fileOpen()
{
    //menu->hide();
    //editBar->hide();
    spreadsheet->hide();
    spreadsheet->spreadsheet->hide();
    setCentralWidget( fileSelector );
    fileSelector->reread();
    fileSelector->raise();
    fileSelector->show();
}

void PIMSpreadsheetWindow::fileClose()
{
    spreadsheet->spreadsheet->save();
    fileOpen();
}

void PIMSpreadsheetWindow::newFile( const DocLnk & f )
{
    closeFileSelector();
    spreadsheet->spreadsheet->newFile( f );
}

void PIMSpreadsheetWindow::setDocument(const QString& fileref)
{
    openFile(DocLnk(fileref));
}

void PIMSpreadsheetWindow::openFile( const DocLnk & f )
{
    closeFileSelector();
    spreadsheet->spreadsheet->openFile( f );
}

void PIMSpreadsheetWindow::closeFileSelector()
{
    fileSelector->hide();
    //menu->show();
    //editBar->show();
    spreadsheet->show();
    setCentralWidget( spreadsheet );
    spreadsheet->spreadsheet->show();
    spreadsheet->raise();
}
