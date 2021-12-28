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
#include "addressbook.h"

#include <qaction.h>
#include <qimage.h>
#include <qmenubar.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qpeapplication.h>
#include <qdialog.h>
#include <qlayout.h>
#include <qfile.h>
#include <qdir.h>
#include <qcopchannel_qws.h>
#include <qdatastream.h>
#include <qwhatsthis.h>

#include "config.h"
#include "global.h"
#include "resource.h"
#include "abentry.h"
#include "abeditor.h"
#include "ablabel.h"
#include "addresssettings.h"

#include "abtable.h"

#include <stdlib.h>

static QString addressbookFilename()
{
    QString filename = QPEApplication::documentDir() + "addressbook";
    return filename;
}

static QString addressbookOldXMLFilename()
{
    QString filename = QPEApplication::documentDir() + "addressbook.xml";
    return filename;
}

static QString addressbookXMLFilename()
{
    QString filename = Global::applicationFileName("addressbook","addressbook.xml");
    return filename;
}

AddressbookWindow::AddressbookWindow( QWidget *parent, const char *name, WFlags f )
    : QMainWindow( parent, name, f ), abEditor(0), bAbEditFirstTime(TRUE)
{
    initFields();

    setCaption( tr("Address Book") );
    setIcon( Resource::loadPixmap( "AddressBook" ) );

    setToolBarsMovable( FALSE );

    // Create Toolbars
    QPixmap newIcon, viewIcon, deleteIcon;
    QPixmap editIcon;

    QToolBar *bar = new QToolBar( this );
    bar->setHorizontalStretchable( TRUE );

    QMenuBar *mbList = new QMenuBar( bar );
    mbList->setMargin( 0 );

    listTools = new QToolBar( this, "list operations" );
    listTools->setLabel( tr( "List Operations" ) );

    viewTools = new QToolBar( this, "view operations" );
    viewTools->setLabel( tr( "View Operations" ) );

    editIcon = Resource::loadPixmap( "edit" );
    new QToolButton( editIcon, "Edit Entry", QString::null,
		     this, SLOT(slotViewEdit()), viewTools, "edit entry" );

    deleteIcon = Resource::loadPixmap( "trash" );
    deleteButton = new QToolButton( deleteIcon, "Delete Entry", QString::null,
				    this, SLOT(slotListDelete()), viewTools, "delete entry" );

    QMenuBar *mbView = new QMenuBar( viewTools );
    mbView->setMargin( 0 );

    newIcon = Resource::loadPixmap( "new" );
    QToolButton *nb = new QToolButton( newIcon, "New Entry", QString::null,
		     this, SLOT(slotListNew()), listTools, "edit entry" );
    QWhatsThis::add( nb, tr("Adds a new entry") );

    QPopupMenu *pmSettings = new QPopupMenu( this );
    mbList->insertItem( tr( "Settings" ), pmSettings );

    QAction *a = new QAction( tr( "Field Settings..."), QString::null, 0, 0 );
    connect( a, SIGNAL( activated() ), this, SLOT( slotSettings() ) );
    a->addTo( pmSettings );

    viewTools->hide();

    // Create Views

    cel = new AbEntryList();

    QFile f1( addressbookXMLFilename() );
    QFile f2( addressbookOldXMLFilename() );

    if ( f1.exists() ) {
        cel->loadXML( addressbookXMLFilename() );
    } else if ( f2.exists() ) {
	cel->loadXML( addressbookOldXMLFilename() );
	QFile::remove(addressbookOldXMLFilename());
    } else {
	cel->load( addressbookFilename() );
	QFile::remove(addressbookOldXMLFilename());
	QFile::remove(addressbookFilename());
    }

    entry = new AbEntry();

    abList = new AbTable( this, "table" );
    abList->setHScrollBarMode( QScrollView::AlwaysOff );
    connect( abList, SIGNAL( empty( bool ) ),
	     this, SLOT( listIsEmpty( bool ) ) );
    connect( abList, SIGNAL( clicked( int, int, int, const QPoint & ) ),
	     this, SLOT( slotListView() ) );
    abList->init( cel );
    abList->sortColumn( 0, TRUE, TRUE );

    abView = new AbLabel( this, "viewer" );
    abView->init( entry  );

    abView->hide();
    setCentralWidget( abList );

#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
    QCopChannel* channel = new QCopChannel( "QPE/Sync", this );
    connect( channel, SIGNAL( received(const QCString &, const QByteArray &) ),
             this, SLOT( syncMessage( const QCString &, const QByteArray &) ) );

    connect( qApp, SIGNAL(appMessage(const QCString&, const QByteArray&)),
	     this, SLOT(appMessage(const QCString&, const QByteArray&)) );
#endif

}

void AddressbookWindow::appMessage(const QCString& msg, const QByteArray& data)
{
    if ( msg == "nextView()" ) {
	// ...
    }
}

void AddressbookWindow::resizeEvent( QResizeEvent *e )
{
  QMainWindow::resizeEvent( e );

  if ( centralWidget() == abList  )
      showList();
  else if ( centralWidget() == abView )
      showView();
}

AddressbookWindow::~AddressbookWindow()
{
    cel->saveXML( addressbookXMLFilename() );
}

void AddressbookWindow::showList()
{
    viewTools->hide();
    listTools->show();
    abView->hide();
    setCentralWidget( abList );
    abList->show();

    int contentsWidth = abList->visibleWidth();
    abList->setColumnWidth( 0, contentsWidth - contentsWidth / 2 );
    abList->setColumnWidth( 1, contentsWidth / 2 );
}

void AddressbookWindow::showView()
{
    listTools->hide();
    abList->hide();
    viewTools->show();
    setCentralWidget( abView );
    abView->show();
}

void AddressbookWindow::slotListNew()
{
    delete entry;
    entry = new AbEntry();
    if ( abEditor )
	abEditor->setEntry( entry );
    abView->init( entry );
    entry->touch();
    editEntry( NewEntry );
}

void AddressbookWindow::slotListView()
{
    if ( !abList->currentEntry() )
      return;
    *entry = *(abList->currentEntry());
    abView->sync();
    showView();
}

void AddressbookWindow::slotListDelete()
{
    abList->deleteCurrentEntry();
    showList();
}

void AddressbookWindow::slotViewBack()
{
    showList();
}

void AddressbookWindow::slotViewEdit()
{
    if ( !bAbEditFirstTime )
	abEditor->setEntry( entry );
    editEntry( EditEntry );
}

void AddressbookWindow::editEntry( EntryMode entryMode )
{
    if ( bAbEditFirstTime ) {
	abEditor = new AbEditor( entry, this, "editor" );
	abEditor->setCaption( tr("Edit Address") );
	bAbEditFirstTime = FALSE;
    }

#if defined(Q_WS_QWS) || defined(_WS_QWS_)
    abEditor->showMaximized();
#endif
    abEditor->slotPage1();
    if ( abEditor->exec() == QDialog::Accepted ) {
	setFocus();
	abEditor->parseName();
	if ( entryMode == NewEntry ) {
	    AbEntry *insertEntry = new AbEntry( *entry );
	    QString key = cel->insert( insertEntry );
	    abList->appendEntry( insertEntry, key );
	} else
	    abList->replaceCurrentEntry( new AbEntry( *entry ));
    }
    showList();
}

void AddressbookWindow::listIsEmpty( bool empty )
{
    if ( !empty ) {
	deleteButton->setEnabled( TRUE );
    }
}

void AddressbookWindow::syncMessage( const QCString &msg, const QByteArray & )
{
    if ( msg == "reload()" ) {
	cel->clear();
	cel->loadXML( addressbookXMLFilename() );
	abList->init( cel );
    }
    else if ( msg == "flush()" ) {
	cel->saveXML( addressbookXMLFilename() );
    }
}

void AddressbookWindow::closeEvent( QCloseEvent *e )
{
    if ( centralWidget() == abView ) {
	showList();
	e->ignore();
    } else
	e->accept();
}

void AddressbookWindow::slotSettings()
{
    AddressSettings frmSettings( this );
#if defined(Q_WS_QWS) || defined(_WS_QWS_)
    frmSettings.showMaximized();
#endif

    if ( frmSettings.exec() == QDialog::Accepted ) {
	if ( !bAbEditFirstTime )
	    abEditor->loadFields();
	abList->loadFields();
	abList->refresh();
    }
}

void AddressbookWindow::initFields()
{
    Config cfg( "AddressBook" );
    int i,
	count;
    QString strField;

    // all the address setting stuff should be done here...
    if ( !QFile::exists( QString(getenv("HOME"))
			 + "/Settings/" + "AddressBook.conf") ) {
	QStringList slFields,
	    slImportFields;
	cfg.setGroup( "AddressFields" );
	
	strField = cfg.readEntry( "Field" + QString::number(0),
				  QString::null );
	if ( strField.isNull() ) {
	    slFields // WORK STUFF
		<< tr( "Title" ) << tr( "Department" ) << tr( "Company" )
		<< tr( "Work Address" ) << tr( "Work City" )
		<< tr( "Work State" ) << tr( "Work Zip" )
		<< tr( "Work Country" )<< tr( "Email" )
		<< tr( "Work Ph" ) << tr( "Work Fax" ) << tr( "Work Mb" )
		<< tr( "Work Pager" ) << tr( "Work WebPage" )
		// HOME STUFF
		<< tr( "Home Street" ) << tr( "Home City" ) << tr( "HomeZip" )
		<< tr( "Home State" ) << tr( "Home Country" )
		<< tr( "Home Ph" ) << tr( "Home Fax" ) << tr( "Home Mb" )
		<< tr( "Home WebPage" )
		// PERSONAL STUFF
		<< tr( "Spouse" ) << tr( "Gender" ) << tr( "Birthday" )
		<< tr( "Aniversary" ) << tr( "Nickname" );
	}
	
	cfg.setGroup( "ImportantCategory" );
	strField = cfg.readEntry( "Category" + QString::number(0),
				  QString::null );
	if ( strField.isNull() ) {
	    slImportFields.append( tr("Work Ph") );
	    slImportFields.append( tr("Work Fax") );
	    slImportFields.append( tr("Work Mb") );
	    slImportFields.append( tr("Email") );
	    slImportFields.append( tr("Home Ph") );
	    slImportFields.append( tr("Title") );
	    slImportFields.append( tr("Company") );
	    slImportFields.append( tr("Work Address") );
	}
	
	count = slImportFields.count();
	for ( i = 0; i < count; i++ )
	    cfg.writeEntry( "Category" + QString::number(i),
			    slImportFields[i] );
	cfg.setGroup( "AddressFields" );
	count = slFields.count();
	for ( i = 0; i < count; i++ )
	    cfg.writeEntry( "Field" + QString::number(i), slFields[i] );
	
    }

    // compatibility... I added this later, so must check outside of
    // file existence...
    bool bFirstTime = FALSE;
    cfg.setGroup( "XMLFields" );
    QStringList slXMLField;
    strField = cfg.readEntry( "XMLField" + QString::number(0),
			      QString::null );
    // if we get one we get them all, at least we had better
    if ( strField.isNull() ) {
	bFirstTime = TRUE;
	slXMLField  // WORK STUFF
	        << "JobTitle" << "Department" <<  "Company"
		<< "BusinessStreet" << "BusinessCity"
		<< "BusinessState" << "BusinessZip"
		<< "BusinessCountry" << "DefaultEmail"
		<< "BusinessPhone" << "BusinessFax"
		<< "BusinessMobile" << "BusinessPager" << "BusinessWebPage"
		// HOME STUFF
		<< "HomeStreet" << "HomeCity" << "HomeZip"
		<< "HomeState" << "HomeCountry"
		<< "HomePhone" << "HomeFax" << "HomeMobile"
		<< "HomeWebPage"
		// PERSONAL STUFF
		<< "Spouse" << "Gender" << "Birthday"
		<< "Aniversary" << "Nickname";
    }
    if ( bFirstTime ) {
	count = slXMLField.count();
	for ( i = 0; i < count; i++ )
	    cfg.writeEntry( "XMLField" + QString::number(i),
			    slXMLField[i] );
	
    }
}
