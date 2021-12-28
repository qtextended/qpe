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

#include "helpbrowser.h"
#include <qstatusbar.h>
#include <qdragobject.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qiconset.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qstylesheet.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qpeapplication.h>
#include <qevent.h>
#include <qlineedit.h>
#include <qobjectlist.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qdatastream.h>
#include <qprinter.h>
#include <qsimplerichtext.h>
#include <qpaintdevicemetrics.h>
#include <qaction.h>
#include <resource.h>

#include <ctype.h>


HelpBrowser::HelpBrowser( QWidget* parent, const char *name, WFlags f )
    : QMainWindow( parent, name, f ),
      selectedURL()
{
    QStringList path;
    path += QPEApplication::qpeDir() + "/help/html";
    path += QPEApplication::qpeDir() + "/docs";
    path += QPEApplication::qpeDir() + "/pics";
    QString dir = QDir::current().canonicalPath();
    if ( dir == "/" )
	dir += "/docs";
    else {
	path += dir + "/../pics";
	dir += "/../docs";
	path += dir;
    }

    init( "index.html", path );
}

class HelpBrowserMimeFactory : public QMimeSourceFactory {
public:
    HelpBrowserMimeFactory(const QStringList& _path)
    {
	setFilePath( _path );
	setExtensionType("html","text/html;charset=UTF-8");
    }

    const QMimeSource* data(const QString& abs_name) const
    {
	const QMimeSource* r = QMimeSourceFactory::data(abs_name);
	if ( !r ) {
	    int sl = abs_name.findRev('/');
	    QString name = sl>=0 ? abs_name.mid(sl+1) : abs_name;
	    int dot = name.findRev('.');
	    if ( dot >= 0 )
		name = name.left(dot);
	    QImage img = Resource::loadImage(name);
	    if ( !img.isNull() )
		r = new QImageDrag(img);
	}
	return r;
    }
};

void HelpBrowser::init( const QString& _home, const QStringList& _path )
{
    readBookmarks();

    setIcon( Resource::loadPixmap( "help_icon" ) );

    browser = new QTextBrowser( this );
    browser->setMimeSourceFactory(new HelpBrowserMimeFactory(_path));
    browser->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    connect( browser, SIGNAL( textChanged() ),
	     this, SLOT( textChanged() ) );

    setCentralWidget( browser );
    setToolBarsMovable( FALSE );

    if ( !_home.isEmpty() )
	browser->setSource( _home );

    QToolBar* toolbar = new QToolBar( this );
    toolbar->setHorizontalStretchable( TRUE );
    QMenuBar *menu = new QMenuBar( toolbar );

    // toolbar = new QToolBar( this );
    // addToolBar( toolbar, "Toolbar");

    QPopupMenu* go = new QPopupMenu( this );
    backAction = new QAction( tr( "Backward" ), Resource::loadPixmap( "back" ), QString::null, 0, this, 0 );
    connect( backAction, SIGNAL( activated() ), browser, SLOT( backward() ) );
    connect( browser, SIGNAL( backwardAvailable( bool ) ),
	     backAction, SLOT( setEnabled( bool ) ) );
    backAction->addTo( go );
    backAction->addTo( toolbar );
    backAction->setEnabled( FALSE );

    forwardAction = new QAction( tr( "Forward" ), Resource::loadPixmap( "forward" ), QString::null, 0, this, 0 );
    connect( forwardAction, SIGNAL( activated() ), browser, SLOT( forward() ) );
    connect( browser, SIGNAL( forwardAvailable( bool ) ),
	     forwardAction, SLOT( setEnabled( bool ) ) );
    forwardAction->addTo( go );
    forwardAction->addTo( toolbar );
    forwardAction->setEnabled( FALSE );

    QAction *a = new QAction( tr( "Home" ), Resource::loadPixmap( "home" ), QString::null, 0, this, 0 );
    connect( a, SIGNAL( activated() ), browser, SLOT( home() ) );
    a->addTo( go );
    a->addTo( toolbar );

    bookm = new QPopupMenu( this );
    bookm->insertItem( tr( "Add Bookmark" ), this, SLOT( addBookmark() ) );
    bookm->insertSeparator();

    QStringList::Iterator it2 = bookmarks.begin();
    for ( ; it2 != bookmarks.end(); ++it2 )
	mBookmarks[ bookm->insertItem( *it2 ) ] = *it2;
    connect( bookm, SIGNAL( activated( int ) ),
	     this, SLOT( bookmChosen( int ) ) );

    menu->insertItem( tr("Go"), go );
    menu->insertItem( tr( "Bookmarks" ), bookm );

    resize( 240, 300 );
    browser->setFocus();

    connect( qApp, SIGNAL(appMessage(const QCString&, const QByteArray&)),
	     this, SLOT(appMessage(const QCString&, const QByteArray&)) );
}

void HelpBrowser::appMessage(const QCString& msg, const QByteArray& data)
{
    if ( msg == "showFile(QString)" ) {
	QDataStream ds(data,IO_ReadOnly);
	QString fn;
	ds >> fn;
	if ( !fn.isEmpty() )
	    browser->setSource( fn );
	raise();
    }
}

void HelpBrowser::textChanged()
{
    if ( browser->documentTitle().isNull() )
	setCaption( tr("Help Browser") );
    else
	setCaption( browser->documentTitle() ) ;

    selectedURL = caption();
}

HelpBrowser::~HelpBrowser()
{
    bookmarks.clear();
    QMap<int, QString>::Iterator it2 = mBookmarks.begin();
    for ( ; it2 != mBookmarks.end(); ++it2 )
	bookmarks.append( *it2 );

    QFile f2( QDir::currentDirPath() + "/.bookmarks" );
    if ( f2.open( IO_WriteOnly ) ) {
	QDataStream s2( &f2 );
	s2 << bookmarks;
	f2.close();
    }
}

void HelpBrowser::pathSelected( const QString &_path )
{
    browser->setSource( _path );
}

void HelpBrowser::readBookmarks()
{
    if ( QFile::exists( QDir::currentDirPath() + "/.bookmarks" ) ) {
	QFile f( QDir::currentDirPath() + "/.bookmarks" );
	if ( f.open( IO_ReadOnly ) ) {
	    QDataStream s( &f );
	    s >> bookmarks;
	    f.close();
	}
    }
}

void HelpBrowser::bookmChosen( int i )
{
    if ( mBookmarks.contains( i ) )
	browser->setSource( mBookmarks[ i ] );
}

void HelpBrowser::addBookmark()
{
    mBookmarks[ bookm->insertItem( caption() ) ] = caption();
}
