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
#include "mainwindow.h"
#include "todotable.h"
#include "todoentryimpl.h"

#include <qpeapplication.h>
#include <organizer.h>
#include <resource.h>
#include <global.h>
#include <config.h>

#include <qtoolbar.h>
#include <qaction.h>
#include <qmessagebox.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qdir.h>
#include <qcopchannel_qws.h>
#include <qdatastream.h>

#include <stdlib.h>

static QString todolistOldXMLFilename()
{
    QString filename = QPEApplication::documentDir() + "todo.xml";
    return filename;
}

static QString todolistXMLFilename()
{
    return Global::applicationFileName("todolist","todolist.xml");
}

static QString categoriesXMLFilename()
{
    return Global::applicationFileName("todolist","categories.xml");
}

TodoWindow::TodoWindow( QWidget *parent, const char *name, WFlags f = 0 ) :
    QMainWindow( parent, name, f )
{
    setIcon( Resource::loadPixmap( "todo_icon" ) );
    setCaption( tr("Todo") );

    table = new TodoTable( this );
    connect( table, SIGNAL( currentChanged( int, int ) ),
             this, SLOT( currentEntryChanged( int, int ) ) );
    table->setColumnWidth( 2, 10 );

    QFile f( todolistXMLFilename() );
    if ( !f.exists() ) {
	table->loadOldFormat( todolistOldXMLFilename() );
	table->save( todolistXMLFilename() );
	QFile::remove( todolistOldXMLFilename() );
    } else {
	table->load( todolistXMLFilename() );
	table->loadCategories( categoriesXMLFilename() );
    }

    setCentralWidget( table );
    setToolBarsMovable( FALSE );

    Config config( "todo" );
    config.setGroup( "View" );
    bool complete = config.readBoolEntry( "ShowComplete", true );
    table->setShowCompleted( complete );
    QString category = config.readEntry( "Category", QString::null );
    table->setShowCategory( category );

    QToolBar *bar = new QToolBar( this );
    bar->setHorizontalStretchable( TRUE );

    QMenuBar *mb = new QMenuBar( bar );

    QPopupMenu *view = new QPopupMenu( this );
    QPopupMenu *edit = new QPopupMenu( this );
    contextMenu = new QPopupMenu( this );

    bar = new QToolBar( this );

    QAction *a = new QAction( tr( "New Task" ), Resource::loadPixmap( "new" ), QString::null, 0, this, 0 );
    connect( a, SIGNAL( activated() ),
             this, SLOT( slotNew() ) );
    a->addTo( bar );

    a = new QAction( tr( "Edit" ), Resource::loadPixmap( "edit" ), QString::null, 0, this, 0 );
    connect( a, SIGNAL( activated() ),
             this, SLOT( slotEdit() ) );
    a->addTo( bar );
    a->addTo( edit );
    a->addTo( contextMenu );
    a->setEnabled( FALSE );
    editAction = a;

    a = new QAction( tr( "Delete" ), Resource::loadPixmap( "cut" ), QString::null, 0, this, 0 );
    connect( a, SIGNAL( activated() ),
             this, SLOT( slotDelete() ) );
    a->addTo( bar );
    a->addTo( edit );
    a->addTo( contextMenu );
    a->setEnabled( FALSE );
    deleteAction = a;

    a = new QAction( QString::null, tr( "Completed tasks" ), 0, this, 0, TRUE );
    connect( a, SIGNAL( toggled(bool) ), this, SLOT( showCompleted(bool) ) );
    a->addTo( view );
    a->setOn( table->showCompleted() );

    catMenu = new QPopupMenu( this );
    catMenu->setCheckable( true );
    connect( catMenu, SIGNAL(activated(int)), this, SLOT(setCategory(int)) );
    populateCategories();

    view->insertItem( tr("Category"), catMenu );

    mb->insertItem( tr( "View" ), view );
    mb->insertItem( tr( "Task" ), edit );

    resize( 200, 300 );
    if ( table->numRows() > 0 )
        currentEntryChanged( 0, 0 );
    connect( table, SIGNAL( signalEdit() ),
             this, SLOT( slotEdit() ) );
    connect( table, SIGNAL(signalShowMenu(const QPoint &)),
	     this, SLOT( slotShowPopup(const QPoint &)) );

#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
    QCopChannel* channel = new QCopChannel( "QPE/Sync", this );
    connect( channel, SIGNAL( received(const QCString &, const QByteArray &) ),
             this, SLOT( syncMessage( const QCString &, const QByteArray &) ) );
#endif
}

void TodoWindow::slotNew()
{
    QStringList categories = table->categories();

    NewTaskDialog e( this, 0, TRUE );
    e.setCategories( categories );
    Todo *todo;

#if defined(Q_WS_QWS) || defined(_WS_QWS_)
    e.showMaximized();
#endif
    int ret = e.exec();

    if ( ret == QDialog::Accepted ) {
	table->setPaintingEnabled( false );
        todo = e.todoEntry();
	if ( categories.find( todo->category() ) == categories.end() ) {
	    table->addCategory( todo->category() );
	    populateCategories();
	}
        table->addEntry( todo );
        table->save( todolistXMLFilename() );
        currentEntryChanged( 0, 0 );
	table->setPaintingEnabled( true );
    }
}

TodoWindow::~TodoWindow()
{
    // go through and update things this is an ugly hack, but it will solve things
    // Some individual ought to create a better way to sync the table with the list...
    int i,
        rows;
    Todo *t;
    rows = table->numRows();
    for ( i = 0; i < rows; i++ ) {
        table->setCurrentCell( i, 0 );
        t = table->currentEntry();
        table->replaceEntry( t );
    }
    table->save( todolistXMLFilename() );
    table->saveCategories( categoriesXMLFilename() );
    Config config( "todo" );
    config.setGroup( "View" );
    config.writeEntry( "ShowComplete", table->showCompleted() );
    config.writeEntry( "Category", table->showCategory() );
}


void TodoWindow::slotDelete()
{
    if ( table->currentRow() == -1 )
        return;
    table->setPaintingEnabled( false );
    table->clearCell( table->currentRow(), 0 );
    table->save( todolistXMLFilename() );
    table->clear();
    table->load( todolistXMLFilename() );
    table->setPaintingEnabled( true );

    if ( table->numRows() == 0 )
        currentEntryChanged( -1, 0 );
}

void TodoWindow::slotEdit()
{
    Todo *todo = table->currentEntry();
    if ( !todo )
        return;

    QStringList categories = table->categories();

    NewTaskDialog e( this, 0, TRUE );
    e.setCaption( tr( "Edit Task" ) );
    e.setCategories( categories );
    e.init( todo );

#if defined(Q_WS_QWS) || defined(_WS_QWS_)
    e.showMaximized();
#endif
    int ret = e.exec();

    if ( ret == QDialog::Accepted ) {
	table->setPaintingEnabled( false );
        todo = e.todoEntry();
	if ( categories.find( todo->category() ) == categories.end() ) {
	    table->addCategory( todo->category() );
	    populateCategories();
	}
        table->replaceEntry( todo );
        table->save( todolistXMLFilename() );
	table->setPaintingEnabled( true );
    }
}

void TodoWindow::slotShowPopup( const QPoint &p )
{
    contextMenu->popup( p );
}

void TodoWindow::showCompleted( bool s )
{
    table->setPaintingEnabled( false );
    table->setShowCompleted( s );
    table->setPaintingEnabled( true );
}

void TodoWindow::currentEntryChanged( int r, int )
{
    if ( r != -1 ) {
        editAction->setEnabled( TRUE );
        deleteAction->setEnabled( TRUE );
    } else {
        editAction->setEnabled( FALSE );
        deleteAction->setEnabled( FALSE );
    }
}

void TodoWindow::setCategory( int c )
{
    table->setPaintingEnabled( false );
    for ( unsigned int i = 0; i < catMenu->count(); i++ )
	catMenu->setItemChecked( i, c == (int)i );
    if ( c == 0 ) {
	table->setShowCategory( QString::null );
	setCaption( tr("Todo") + " - " + tr("All") );
    } else {
	QString cat = table->categories()[c-1];
	table->setShowCategory( cat );
	setCaption( tr("Todo") + " - " + cat );
    }
    table->setPaintingEnabled( true );
}

void TodoWindow::populateCategories()
{
    catMenu->clear();
    int id = 0;
    catMenu->insertItem( tr("All"), id++ );
    catMenu->insertSeparator();
    QStringList categories = table->categories();
    for ( QStringList::Iterator it = categories.begin(); it != categories.end(); ++it ) {
	catMenu->insertItem( *it, id );
	if ( *it == table->showCategory() )
	    setCategory( id );
	++id;
    }
    if ( table->showCategory().isEmpty() )
	setCategory( 0 );
}

void TodoWindow::syncMessage( const QCString &msg, const QByteArray & )
{
    if ( msg == "reload()" ) {
	table->clear();
	table->loadCategories( categoriesXMLFilename() );
	table->load( todolistXMLFilename() );
    }
    else if ( msg == "flush()" ) {
	table->save( todolistXMLFilename() );
    }
}
