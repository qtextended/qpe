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
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <qmessagebox.h>
#include <qdir.h>
#include <qregexp.h>
#include <qheader.h>
#include <qtoolbar.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qaction.h>
#include <qstringlist.h>
#include <qcursor.h>
#include <qmultilineedit.h>
#include <qfont.h>
#include <resource.h>
#include <global.h>

#include "inlineedit.h"
#include "filebrowser.h"
#include "mimetype.h"
#include "applnk.h"

//
//  FileItem
//
FileItem::FileItem( QListView * parent, const QFileInfo & fi )
    : QListViewItem( parent ),
      fileInfo( fi )
{
    QDate d = fi.lastModified().date();

    setText( 0, fi.fileName() );
    setText( 1, sizeString( fi.size() ) + " " );
    setText( 2, QString().sprintf("%4d-%02d-%02d",d.year(), d.month(), d.day() ) );

    DocLnk doc( fi.filePath() );

    if( fi.isDir() )
	setText( 3, "directory" );
    else if( isLib() )
	setText( 3, "library" );
    else
	setText( 3, doc.type() );

    if( fi.isDir() ){
	if( !QDir( fi.filePath() ).isReadable() )
	    setPixmap( 0, Resource::loadPixmap( "lockedfolder" ) );
	else
	    setPixmap( 0, Resource::loadPixmap( "folder" ) );
    }
    else if( !fi.isReadable() )
	setPixmap( 0, Resource::loadPixmap( "locked" ) );
    else if( isLib() )
	setPixmap( 0 , Resource::loadPixmap( "library" ) );
    else
	setPixmap( 0 , doc.pixmap() );
}

QString FileItem::sizeString( unsigned int s )
{
    double size = s;

    if ( size > 1024 * 1024 * 1024 )
	return QString().sprintf( "%.1f", size / ( 1024 * 1024 * 1024 ) ) + "G";
    else if ( size > 1024 * 1024 )
	return QString().sprintf( "%.1f", size / ( 1024 * 1024 ) ) + "M";
    else if ( size > 1024 )
	return QString().sprintf( "%.1f", size / ( 1024 ) ) + "K";
    else
	return QString::number( size ) + "B";
}

QString FileItem::key( int column, bool ascending ) const
{
    QString tmp;

    ascending = ascending;

    if( (column == 0) && fileInfo.isDir() ){ // Sort by name
	// We want the directories to appear at the top of the list
	tmp = (char) 0;
	return (tmp + text( column ).lower());
    }
    else if( column == 2 ) { // Sort by date
	QDateTime epoch( QDate( 1980, 1, 1 ) );
        tmp.sprintf( "%08d", epoch.secsTo( fileInfo.lastModified() ) );
	return tmp;
    }
    else if( column == 1 ) { // Sort by size
	return tmp.sprintf( "%08d", fileInfo.size() );
    }

    return text( column ).lower();
}

bool FileItem::isLib()
{
    // This is of course not foolproof
    if( !qstrncmp("lib", fileInfo.baseName(), 3) &&
	( fileInfo.extension().contains( "so" ) ||
	  fileInfo.extension().contains( "a" ) ) )
	return TRUE;
    else
	return FALSE;
}

int FileItem::launch()
{
    DocLnk doc( fileInfo.filePath() );
    doc.execute();
    listView()->clearSelection();
    return 1;
}

bool FileItem::rename( const QString & name )
{
    QString oldpath, newpath;

    oldpath = fileInfo.filePath();
    newpath = fileInfo.dirPath() + "/" + name;

    if( ::rename( (const char *) oldpath, (const char *) newpath ) != 0 )
	return FALSE;
    else
	return TRUE;
}

//
//  FileView
//
FileView::FileView( const QString & dir, QWidget * parent,
		    const char * name )
    : QListView( parent, name ),
      menuTimer( this ),
      le( NULL ),
      itemToRename( NULL )
{
    addColumn( "Name" );
    addColumn( "Date" );
    addColumn( "Size" );
    addColumn( "Type" );

    setMultiSelection( TRUE );
    header()->hide();

    setColumnWidthMode( 0, Manual );
    setColumnWidthMode( 3, Manual );

    // right align yize column
    setColumnAlignment( 1, AlignRight );

    generateDir( dir );

    connect( this, SIGNAL( clicked( QListViewItem * )),
	     SLOT( itemClicked( QListViewItem * )) );
    connect( this, SIGNAL( doubleClicked( QListViewItem * )),
	     SLOT( itemDblClicked( QListViewItem * )) );
    connect( this, SIGNAL( selectionChanged() ), SLOT( cancelMenuTimer() ) );
    connect( &menuTimer, SIGNAL( timeout() ), SLOT( showFileMenu() ) );
}

void FileView::resizeEvent( QResizeEvent* )
{
    setColumnWidth( 0, width() - 2 * lineWidth() - 20 - columnWidth( 1 ) - columnWidth( 2 ) );

    // hide type column, we use it for "sort by type" only
    setColumnWidth( 3, 0 );
}

void FileView::updateDir()
{
    generateDir( currentDir );
}

void FileView::setDir( const QString & dir )
{
    dirHistory += currentDir;
    generateDir( dir );
}

void FileView::generateDir( const QString & dir )
{
    QDir d( dir );

    if( d.exists() && !d.isReadable() ) return;

    currentDir = d.canonicalPath();

    d.setFilter( QDir::Dirs | QDir::Files | QDir::NoSymLinks );
    d.setSorting( QDir::Name | QDir::DirsFirst | QDir::IgnoreCase |
	          QDir::Reversed );

    const QFileInfoList * list = d.entryInfoList();
    QFileInfoListIterator it( *list );
    QFileInfo *fi;

    clear();
    while( (fi = it.current()) ){
	if( (fi->fileName() == ".") || (fi->fileName() == "..") ){
	    ++it;
	    continue;
	}
	(void) new FileItem( (QListView *) this, *fi );
	++it;
    }

    emit dirChanged();
}

void FileView::rename()
{
    itemToRename = (FileItem *) currentItem();
    const QPixmap * pm;
    int pmw;

    if( itemToRename == NULL ) return;

    if( ( pm = itemToRename->pixmap( 0 ) ) == NULL )
	pmw = 0;
    else
	pmw = pm->width();

    ensureItemVisible( itemToRename );
    horizontalScrollBar()->setValue( 0 );
    horizontalScrollBar()->setEnabled( FALSE );
    verticalScrollBar()->setEnabled( FALSE );

    selected = isSelected( itemToRename );
    setSelected( itemToRename, FALSE );

    if( le == NULL ){
	le = new InlineEdit( this );
	le->setFrame( FALSE );
	connect( le, SIGNAL( lostFocus() ), SLOT( endRenaming() ) );
    }

    QRect r = itemRect( itemToRename );
    r.setTop( r.top() + frameWidth() + 1 );
    r.setLeft( r.left() + frameWidth() + pmw );
    r.setBottom( r.bottom() + frameWidth() );
    r.setWidth( columnWidth( 0 ) - pmw );

    le->setGeometry( r );
    le->setText( itemToRename->text( 0 ) );
    le->selectAll();
    le->show();
    le->setFocus();
}

void FileView::endRenaming()
{
    if( le && itemToRename ){
	le->hide();
	setSelected( itemToRename, selected );

	if( !itemToRename->rename( le->text() ) ){
	    QMessageBox::warning( this, tr( "Rename file" ),
				  tr( "Rename failed!" ), tr( "&Ok" ) );
	} else {
	    updateDir();
	}
	itemToRename = NULL;
	horizontalScrollBar()->setEnabled( TRUE );
	verticalScrollBar()->setEnabled( TRUE );
    }
}

void FileView::copy()
{
    FileItem * i;

    if((i = (FileItem *) firstChild()) == 0) return;

    flist.clear();
    while( i ){
	if( i->isSelected() /*&& !i->isDir()*/ ){
	    flist += i->getFilePath();
	}
	i = (FileItem *) i->nextSibling();
    }
}

void FileView::paste()
{
    int i, err;
    QString cmd, dest, basename, cd = currentDir;

    if(cd == "/") cd = "";

    for ( QStringList::Iterator it = flist.begin(); it != flist.end(); ++it ) {
	basename = (*it).mid((*it).findRev("/") + 1, (*it).length());

	dest = cd + "/" + basename;
	if( QFile( dest ).exists() ){
	    i = 1;
	    dest = cd + "/Copy of " + basename;
	    while( QFile( dest ).exists() ){
		dest.sprintf( "%s/Copy (%d) of %s", (const char *) cd, i++,
			      (const char *) basename );
	    }
	}

	//
	// Copy a directory recursively using the "cp" command -
	// may have to be changed
	//
	if( QFileInfo( (*it) ).isDir() ){
	    cmd = "/bin/cp -fpR \"" + (*it) +"\" " + "\"" + dest + "\"";
	    err = system( (const char *) cmd );
	} else if( !copyFile( dest, (*it) ) ){
	    err = -1;
	} else {
	    err = 0;
	}

	if( ( err < 0 ) || ( err == 127 ) ){
	    QMessageBox::warning( this, tr("Paste file"), tr("Paste failed!"),
				  tr("&Ok") );
	    break;
	} else {
	    updateDir();
	    QListViewItem * i = firstChild();
	    basename = dest.mid( dest.findRev("/") + 1, dest.length() );

	    while( i ){
		if( i->text(0) == basename ){
		    setCurrentItem( i );
		    ensureItemVisible( i );
		    break;
		}
		i = i->nextSibling();
	    }
	}
    }
}

bool FileView::copyFile( const QString & dest, const QString & src )
{
    char bf[ 50000 ];
    int  bytesRead;
    bool success = TRUE;
    struct stat status;

    QFile s( src );
    QFile d( dest );

    if( s.open( IO_ReadOnly | IO_Raw ) &&
	d.open( IO_WriteOnly | IO_Raw ) )
    {
	while( (bytesRead = s.readBlock( bf, sizeof( bf ) )) ==
	       sizeof( bf ) )
	{
	    if( d.writeBlock( bf, sizeof( bf ) ) != sizeof( bf ) ){
		success = FALSE;
		break;
	    }
	}
	if( success && (bytesRead > 0) ){
	    d.writeBlock( bf, bytesRead );
	}
    } else {
	success = FALSE;
    }

    // Set file permissions
    if( stat( (const char *) src, &status ) == 0 ){
	chmod( (const char *) dest, status.st_mode );
    }

    return success;
}

void FileView::del()
{
    FileItem * i;
    QStringList fl;
    QString cmd;
    int err;

    if((i = (FileItem *) firstChild()) == 0) return;

    while( i ){
	if( i->isSelected() ){
	    fl += i->getFilePath();
	}
	i = (FileItem *) i->nextSibling();
    }
    if( fl.count() < 1 ) return;

    if( QMessageBox::warning( this, tr("Cut"), tr("Are you sure?"),
			      tr("&Yes"), tr("&No") ) == 0)
    {
	//
	// Dependant upon the "rm" command - will probably have to be replaced
	//
	for ( QStringList::Iterator it = fl.begin(); it != fl.end(); ++it ) {
	    cmd = "/bin/rm -rf \"" + (*it) + "\"";
	    err = system( (const char *) cmd );
	    if( ( err < 0 ) || ( err == 127 ) ){
		QMessageBox::warning( this, tr("Cut"), tr("Cut failed!"),
				      tr("&Ok") );
		break;
	    }
	}
	updateDir();
    }
}

void FileView::newFolder()
{
    int t = 1;
    FileItem * i;
    QString nd = currentDir + "/NewFolder";

    while( QFile( nd ).exists() ){
	nd.sprintf( "%s/NewFolder (%d)", (const char *) currentDir, t++ );
    }

    if( mkdir( (const char *) nd, 0777 ) != 0){
	QMessageBox::warning( this, tr( "New folder" ),
			      tr( "Folder creation failed!" ),
			      tr( "&Ok" ) );
	return;
    }
    updateDir();

    if((i = (FileItem *) firstChild()) == 0) return;

    while( i ){
	if( i->isDir() && ( i->getFilePath() == nd ) ){
	    setCurrentItem( i );
	    rename();
	    break;
	}
	i = (FileItem *) i->nextSibling();
    }
}

void FileView::viewAsText()
{
    FileItem * i = (FileItem *) currentItem();
    Global::execute( "textedit -f " + i->getFilePath() );
}

void FileView::itemClicked( QListViewItem * i)
{
    FileItem * t = (FileItem *) i;

    if( t == NULL ) return;
    if( t->isDir() ){
	setDir( t->getFilePath() );
    }
}

void FileView::itemDblClicked( QListViewItem * i)
{
    FileItem * t = (FileItem *) i;

    if(t == NULL) return;
    if(t->launch() == -1){
	QMessageBox::warning( this, tr( "Launch Application" ),
			      tr( "Launch failed!" ), tr( "&Ok" ) );
    }
}

void FileView::parentDir()
{
    setDir( currentDir + "./.." );
}

void FileView::lastDir()
{
    if( dirHistory.count() == 0 ) return;

    generateDir( dirHistory.last() );
    dirHistory.remove( dirHistory.last() );
}

void FileView::contentsMousePressEvent( QMouseEvent * e )
{
    QListView::contentsMousePressEvent( e );
    menuTimer.start( 750, TRUE );
}

void FileView::contentsMouseReleaseEvent( QMouseEvent * e )
{
    QListView::contentsMouseReleaseEvent( e );
    menuTimer.stop();
}

void FileView::cancelMenuTimer()
{
    if( menuTimer.isActive() )
	menuTimer.stop();
}

void FileView::addToDocuments()
{
    FileItem * i = (FileItem *) currentItem();
    DocLnk f;
    QString n = i->text(0);
    n.replace(QRegExp("\\..*"),"");
    f.setName( n );
    f.setFile( i->getFilePath() );
    f.writeLink();
}

void FileView::run()
{
    FileItem * i = (FileItem *) currentItem();
    i->launch();
}

void FileView::showFileMenu()
{
    FileItem * i = (FileItem *) currentItem();
    QPopupMenu * m = new QPopupMenu( this );

    if ( !i->isDir() ) {
	m->insertItem( tr( "Add to Documents" ), this, SLOT( addToDocuments() ) );
	m->insertSeparator();
    }

    MimeType mt(i->getFilePath());
    const AppLnk* app = mt.application();

    if ( !i->isDir() ) {
	if ( app )
	    m->insertItem( app->pixmap(), tr( "&Open in " + app->name() ), this, SLOT( run() ) );
	else if( i->isExecutable() )
	    m->insertItem( Resource::loadPixmap( i->text( 0 ) ), tr( "&Run" ), this, SLOT( run() ) );

	m->insertItem( Resource::loadPixmap( "txt" ), tr( "&View as text" ),
		       this, SLOT( viewAsText() ) );

	m->insertSeparator();
    }

    m->insertItem( tr( "R&ename" ), this, SLOT( rename() ) );
    m->insertItem( Resource::loadPixmap("copy"),
		   tr( "&Copy" ), this, SLOT( copy() ) );
    m->insertItem( Resource::loadPixmap("paste"),
		   tr( "&Paste" ), this, SLOT( paste() ) );
    m->insertItem( Resource::loadPixmap("cut"),
		   tr( "&Cut" ), this, SLOT( del() ) );
    m->insertSeparator();
    m->insertItem( tr( "&Select all" ), this, SLOT( selectAll() ) );
    m->insertItem( tr( "Dese&lect all" ), this, SLOT( deselectAll() ) );
    m->popup( QCursor::pos() );
}

//
//  FileBrowser
//

FileBrowser::FileBrowser( QWidget * parent,
			  const char * name, WFlags f ) :
    QMainWindow( parent, name, f )
{
    init( QDir::current().canonicalPath() );
}

FileBrowser::FileBrowser( const QString & dir, QWidget * parent,
			  const char * name, WFlags f ) :
    QMainWindow( parent, name, f )
{
    init( dir );
}

void FileBrowser::init(const QString & dir)
{
    setCaption( tr("File Manager") );
    setIcon( Resource::loadPixmap( "filebrowser_icon" ) );

    fileView = new FileView( dir, this );
    fileView->setAllColumnsShowFocus( TRUE );

    setCentralWidget( fileView );
    setToolBarsMovable( FALSE );

    QToolBar* toolBar = new QToolBar( this );
    toolBar->setHorizontalStretchable( TRUE );

    QMenuBar* menuBar = new QMenuBar( toolBar );

    dirMenu = new QPopupMenu( this );
    menuBar->insertItem( tr( "&Dir" ), dirMenu );

    sortMenu = new QPopupMenu( this );
    menuBar->insertItem( tr( "&Sort" ), sortMenu );
    sortMenu->insertItem( tr( "by &Name "), this, SLOT( sortName() ) );
    sortMenu->insertItem( tr( "by &Size "), this, SLOT( sortSize() ) );
    sortMenu->insertItem( tr( "by &Date "), this, SLOT( sortDate() ) );
    sortMenu->insertItem( tr( "by &Type "), this, SLOT( sortType() ) );
    sortMenu->insertSeparator();
    sortMenu->insertItem( tr( "Ascending" ), this, SLOT( updateSorting() ) );
    sortMenu->setItemChecked( sortMenu->idAt( 5 ), TRUE );
    sortMenu->setItemChecked( sortMenu->idAt( 0 ), TRUE );

    QAction* a = new QAction( tr("Previous dir"), Resource::loadPixmap( "back" ),
			      QString::null, 0, this, 0 );
    connect( a, SIGNAL( activated() ), fileView, SLOT( lastDir() ) );
    a->addTo( toolBar );

    a = new QAction( tr("Parent dir"), Resource::loadPixmap( "up" ),
		     QString::null, 0, this, 0 );
    connect( a, SIGNAL( activated() ), fileView, SLOT( parentDir() ) );
    a->addTo( toolBar );

    a = new QAction( tr("New folder"), Resource::loadPixmap( "newfolder" ),
		     QString::null, 0, this, 0 );
    connect( a, SIGNAL( activated() ), fileView, SLOT( newFolder() ) );
    a->addTo( toolBar );

    a = new QAction( tr("Copy"), Resource::loadPixmap( "copy" ),
		     QString::null, 0, this, 0 );
    connect( a, SIGNAL( activated() ), fileView, SLOT( copy() ) );
    a->addTo( toolBar );

    a = new QAction( tr("Cut"), Resource::loadPixmap( "cut" ),
		     QString::null, 0, this, 0 );
    connect( a, SIGNAL( activated() ), fileView, SLOT( del() ) );
    a->addTo( toolBar );

    pasteAction = new QAction( tr("Paste"), Resource::loadPixmap( "paste" ),
			       QString::null, 0, this, 0 );
    connect( pasteAction, SIGNAL( activated() ), fileView, SLOT( paste() ) );
    pasteAction->addTo( toolBar );

    connect( fileView, SIGNAL( dirChanged() ), SLOT( updateDirMenu() ) );
    updateDirMenu();
}

void FileBrowser::dirSelected( int id )
{
    int i = 0, j;
    QString dir;

    // Bulid target dir from menu
    while( (j = dirMenu->idAt( i )) != id ){
	dir += dirMenu->text( j ).stripWhiteSpace();
	if( dirMenu->text( j ) != "/" ) dir += "/";
	i++;
    }
    dir += dirMenu->text( dirMenu->idAt( i ) ).stripWhiteSpace();

    fileView->setDir( dir );
}

void FileBrowser::updateDirMenu()
{
    QString spc, cd = fileView->cd();
    QStringList l = QStringList::split( "/", cd );
    int i = 0;

    dirMenu->clear();
    dirMenu->insertItem( tr( "/" ), this, SLOT( dirSelected(int) ) );

    for ( QStringList::Iterator it = l.begin(); it != l.end(); ++it ) {
	spc.fill( ' ', i++);
	dirMenu->insertItem( spc + (*it), this,
			     SLOT( dirSelected(int) ) );
    }
    dirMenu->setItemChecked( dirMenu->idAt( l.count() ), TRUE );
}

void FileBrowser::sortName()
{
    fileView->setSorting( 0, sortMenu->isItemChecked( sortMenu->idAt( 5 ) ) );
    fileView->sort();
    sortMenu->setItemChecked( sortMenu->idAt( 0 ), TRUE );
    sortMenu->setItemChecked( sortMenu->idAt( 1 ), FALSE );
    sortMenu->setItemChecked( sortMenu->idAt( 2 ), FALSE );
    sortMenu->setItemChecked( sortMenu->idAt( 3 ), FALSE );
}

void FileBrowser::sortSize()
{
    fileView->setSorting( 1, sortMenu->isItemChecked( sortMenu->idAt( 5 ) ) );
    fileView->sort();
    sortMenu->setItemChecked( sortMenu->idAt( 0 ), FALSE );
    sortMenu->setItemChecked( sortMenu->idAt( 1 ), TRUE );
    sortMenu->setItemChecked( sortMenu->idAt( 2 ), FALSE );
    sortMenu->setItemChecked( sortMenu->idAt( 3 ), FALSE );
}

void FileBrowser::sortDate()
{
    fileView->setSorting( 2, sortMenu->isItemChecked( sortMenu->idAt( 5 ) ) );
    fileView->sort();
    sortMenu->setItemChecked( sortMenu->idAt( 0 ), FALSE );
    sortMenu->setItemChecked( sortMenu->idAt( 1 ), FALSE );
    sortMenu->setItemChecked( sortMenu->idAt( 2 ), TRUE );
    sortMenu->setItemChecked( sortMenu->idAt( 3 ), FALSE );
}

void FileBrowser::sortType()
{
    fileView->setSorting( 3, sortMenu->isItemChecked( sortMenu->idAt( 5 ) ) );
    fileView->sort();
    sortMenu->setItemChecked( sortMenu->idAt( 0 ), FALSE );
    sortMenu->setItemChecked( sortMenu->idAt( 1 ), FALSE );
    sortMenu->setItemChecked( sortMenu->idAt( 2 ), FALSE );
    sortMenu->setItemChecked( sortMenu->idAt( 3 ), TRUE );
}

void FileBrowser::updateSorting()
{
    sortMenu->setItemChecked( sortMenu->idAt( 5 ), !sortMenu->isItemChecked( sortMenu->idAt( 5 ) ) );

    if ( sortMenu->isItemChecked( sortMenu->idAt( 0 ) ) )
	sortName();
    else if ( sortMenu->isItemChecked( sortMenu->idAt( 1 ) ) )
	sortSize();
    else if ( sortMenu->isItemChecked( sortMenu->idAt( 2 ) ) )
	sortDate();
    else
	sortType();
}
