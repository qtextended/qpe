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
#include "fileselector.h"
#include "global.h"
#include "resource.h"
#include "config.h"
#include "applnk.h"

#include <stdlib.h>

#include <qdir.h>
#include <qwidget.h>
#include <qpopupmenu.h>
#include <qtoolbutton.h>
#include <qpushbutton.h>
#include <qheader.h>
#include <qtooltip.h>

FileSelectorItem::FileSelectorItem( QListView *parent, const DocLnk &f )
    : QListViewItem( parent ), fl( f )
{
    setText( 0, f.name() );
    setPixmap( 0, f.pixmap() );
}

FileSelectorItem::~FileSelectorItem()
{
}

FileSelectorView::FileSelectorView( const QString &f, QWidget *parent, const char *name )
    : QListView( parent, name ), filter( f ), count( 0 )
{
    setAllColumnsShowFocus( TRUE );
    addColumn( tr( "Name" ) );
    header()->hide();

    fileManager = new FileManager;
    reread();
}

FileSelectorView::~FileSelectorView()
{
    
}

void FileSelectorView::reread()
{
    clear();
    QString dir = QString(getenv("HOME")) + "/Documents";
    DocLnkSet files( dir, filter );
    count = files.children().count();
    QListIterator<DocLnk> it( files.children() );
    for ( ; it.current(); ++it )
        (void)new FileSelectorItem( this, **it );
}

/*!
  \class FileSelector fileselector.h
  \brief The FileSelector widget allows the user to select DocLnk objects.
*/

/*!
  Constructs a FileSelector with mime filter \a f.
  The standard Qt \a parent and \a name parameters are passed to the
  parent.

  If \a newVisible is TRUE, the widget has an button allowing the user
  the create "new" documents - editor applications will have this while
  viewer applications will not.

  If \a closeVisible is TRUE, the widget has an button allowinf the user
  to select "no document".

  \sa DocLnkSet::DocLnkSet()
*/
FileSelector::FileSelector( const QString &f, QWidget *parent, const char *name, bool newVisible, bool closeVisible )
    : QVBox( parent, name ), filter( f )
{
    setMargin( 0 );
    setSpacing( 0 );
    QHBox *top = new QHBox( this );
    top->setBackgroundMode( PaletteButton );	// same colour as toolbars
    top->setSpacing( 0 );

    QWidget *spacer = new QWidget( top );
    spacer->setBackgroundMode( PaletteButton );

    QToolButton *tb = new QToolButton( top );
    tb->setPixmap( Resource::loadPixmap( "new" ) );
    connect( tb, SIGNAL( clicked() ), this, SLOT( createNew() ) );
    buttonNew = tb;
    tb->setFixedSize( 18, 20 ); // tb->sizeHint() );
    tb->setAutoRaise( TRUE );
    QToolTip::add( tb, tr( "Create a new Document" ) );
    
    tb = new QToolButton( top );
    tb->setPixmap( Resource::loadPixmap( "close" ) );
    connect( tb, SIGNAL( clicked() ), this, SIGNAL( closeMe() ) );
    buttonClose = tb;
    tb->setFixedSize( 18, 20 ); // tb->sizeHint() );
    tb->setAutoRaise( TRUE );
    QToolTip::add( tb, tr( "Close the File Selector" ) );

    view = new FileSelectorView( filter, this, "fileview" );
    connect( view, SIGNAL( clicked( QListViewItem * ) ),
	     this, SLOT( fileClicked( QListViewItem * ) ) );
    connect( view, SIGNAL( returnPressed( QListViewItem * ) ),
	     this, SLOT( fileClicked( QListViewItem * ) ) );

    setNewVisible( newVisible );
    setCloseVisible( closeVisible );
}

/*!
  Destroys the widget.
*/
FileSelector::~FileSelector()
{
    
}

/*!
  Returns the number of files in the view. If this is zero, and editor
  application might avoid using the selector and immediately start with
  a "new" document.
*/
int FileSelector::fileCount()
{
    return view->fileCount();
}

/*!
  Causes the file selector to act as if the "new" button was chosen.

  \sa newSelected(), closeMe()
*/
void FileSelector::createNew()
{
    DocLnk f;
    emit newSelected( f );
    emit closeMe();
}

void FileSelector::fileClicked( QListViewItem *i )
{
    if ( !i )
	return;
    emit fileSelected( ( (FileSelectorItem*)i )->file() );
    emit closeMe();
}

/*!
  Returns the selected DocLnk. The caller is responsible for deleting
  the returned value.
*/
const DocLnk *FileSelector::selected()
{
    FileSelectorItem *item = (FileSelectorItem *)view->selectedItem();
    if ( item )
	return new DocLnk( item->file() );
    return NULL;
}

/*!
  \fn void FileSelector::fileSelected( const DocLnk &f )

  This signal is emitted when the user selects a file.
  \a f is the file.
*/

/*!
  \fn void FileSelector::newSelected( const DocLnk &f )

  This signal is emitted when the user selects "new" file.
  \a f is a DocLnk for the file. You will need to set the type
  of the value after copying it.
*/

/*!
  \fn void FileSelector::closeMe()

  This signal is emitted when the user no longer needs to view the widget.
*/


/*!
  Sets whether a "new document" button is visible, according to \a b.
*/
void FileSelector::setNewVisible( bool b )
{
    if ( b )
	buttonNew->show();
    else
	buttonNew->hide();
}

/*!
  Sets whether a "no document" button is visible, according to \a b.
*/
void FileSelector::setCloseVisible( bool b )
{
    if ( b )
	buttonClose->show();
    else
	buttonClose->hide();
}

/*!
  Rereads the list of files.
*/
void FileSelector::reread()
{
    view->reread();
}


