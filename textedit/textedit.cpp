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
#include "textedit.h"
#include "fileselector.h"
#include "applnk.h"

#include <qwidgetstack.h>
#include <qmenubar.h>
#include <qtoolbar.h>
#include <qpopupmenu.h>
#include <qaction.h>
#include <qapplication.h>
#include <qlineedit.h>
#include <qtoolbutton.h>
#include <qcolordialog.h>
#include <qspinbox.h>
#include <qobjectlist.h>
#include <global.h>
#include <qfileinfo.h>

#include <stdlib.h> //getenv

#include "resource.h"

static int u_id = 1;
static int get_unique_id()
{
    return u_id++;
}


TextEdit::TextEdit( QWidget *parent, const char *name, WFlags f )
    : QMainWindow( parent, name, f ), bFromDocView( FALSE )
{
    doc = 0;

    QString lang = getenv( "LANG" );

    setToolBarsMovable( FALSE );

    setIcon( Resource::loadPixmap( "TextEditor" ) );

    QToolBar *bar = new QToolBar( this );
    bar->setHorizontalStretchable( TRUE );
    menu = bar;

    QMenuBar *mb = new QMenuBar( bar );
    QPopupMenu *file = new QPopupMenu( this );
    QPopupMenu *edit = new QPopupMenu( this );

//    bar = new QToolBar( this );
    editBar = bar;

    QAction *a = new QAction( tr( "New" ), Resource::loadPixmap( "new" ), QString::null, 0, this, 0 );
    connect( a, SIGNAL( activated() ), this, SLOT( fileNew() ) );
    a->addTo( bar );
    a->addTo( file );

    a = new QAction( tr( "Open" ), Resource::loadPixmap( "fileopen" ), QString::null, 0, this, 0 );
    connect( a, SIGNAL( activated() ), this, SLOT( fileOpen() ) );
    a->addTo( bar );
    a->addTo( file );

    a = new QAction( tr( "Revert" ), Resource::loadPixmap( "close" ), QString::null, 0, this, 0 );
    connect( a, SIGNAL( activated() ), this, SLOT( fileRevert() ) );
    a->addTo( file );

    a = new QAction( tr( "Cut" ), Resource::loadPixmap( "cut" ), QString::null, 0, this, 0 );
    connect( a, SIGNAL( activated() ), this, SLOT( editCut() ) );
    a->addTo( editBar );
    a->addTo( edit );

    a = new QAction( tr( "Copy" ), Resource::loadPixmap( "copy" ), QString::null, 0, this, 0 );
    connect( a, SIGNAL( activated() ), this, SLOT( editCopy() ) );
    a->addTo( editBar );
    a->addTo( edit );

    a = new QAction( tr( "Paste" ), Resource::loadPixmap( "paste" ), QString::null, 0, this, 0 );
    connect( a, SIGNAL( activated() ), this, SLOT( editPaste() ) );
    a->addTo( editBar );
    a->addTo( edit );

//     a = new QAction( tr( "Find..." ), Resource::loadPixmap( "find" ), QString::null, 0, this, 0 );
//     connect( a, SIGNAL( activated() ), this, SLOT( editFind() ) );
//     edit->insertSeparator();
//     a->addTo( bar );
//     a->addTo( edit );

    mb->insertItem( tr( "File" ), file );
    mb->insertItem( tr( "Edit" ), edit );

    searchBar = new QToolBar( "Search", this, QMainWindow::Top, TRUE );

    searchBar->setHorizontalStretchable( TRUE );

    searchEdit = new QLineEdit( searchBar, "searchEdit" );
    searchBar->setStretchableWidget( searchEdit );
    connect( searchEdit, SIGNAL( textChanged( const QString & ) ),
	     this, SLOT( search() ) );

//     a = new QAction( tr( "Find Next" ), Resource::loadPixmap( "next" ), QString::null, 0, this, 0 );
//     connect( a, SIGNAL( activated() ), this, SLOT( findNext() ) );
//     a->addTo( searchBar );
//     a->addTo( edit );

    a = new QAction( tr( "Close Find" ), Resource::loadPixmap( "close" ), QString::null, 0, this, 0 );
    connect( a, SIGNAL( activated() ), this, SLOT( findClose() ) );
    a->addTo( searchBar );

    searchBar->hide();
		
    editorStack = new QWidgetStack( this );
    setCentralWidget( editorStack );

    searchVisible = FALSE;

    fileSelector = new FileSelector( "text/*", editorStack, "fileselector" );
    connect( fileSelector, SIGNAL( closeMe() ), this, SLOT( showEditTools() ) );
    connect( fileSelector, SIGNAL( newSelected( const DocLnk &) ), this, SLOT( newFile( const DocLnk & ) ) );
    connect( fileSelector, SIGNAL( fileSelected( const DocLnk &) ), this, SLOT( openFile( const DocLnk & ) ) );
    fileOpen();

    editor = new QMultiLineEdit( editorStack );
    editorStack->addWidget( editor, get_unique_id() );

    resize( 200, 300 );
    // don't need the close visible, it is redundant...
    fileSelector->setCloseVisible( FALSE );
}

TextEdit::~TextEdit()
{
    save();
}

void TextEdit::fileNew()
{
    save();
    newFile(DocLnk());
}

void TextEdit::fileOpen()
{
    save();
    menu->hide();
    editBar->hide();
    searchBar->hide();
    editorStack->raiseWidget( fileSelector );
    fileSelector->reread();
    updateCaption();
}

void TextEdit::fileRevert()
{
    clear();
    fileOpen();
}

void TextEdit::editCut()
{
#ifndef QT_NO_CLIPBOARD
    editor->cut();
#endif
}

void TextEdit::editCopy()
{
#ifndef QT_NO_CLIPBOARD
    editor->copy();
#endif
}

void TextEdit::editPaste()
{
#ifndef QT_NO_CLIPBOARD
    editor->paste();
#endif
}

void TextEdit::editFind()
{
    searchBar->show();
    searchVisible = TRUE;
    searchEdit->setFocus();
}

void TextEdit::findNext()
{
}

void TextEdit::findClose()
{
    searchVisible = FALSE;
    searchBar->hide();
}

void TextEdit::search()
{
}

void TextEdit::newFile( const DocLnk &f )
{
    DocLnk nf = f;
    nf.setType("text/plain");
    clear();
    editorStack->raiseWidget( editor );
    editor->setFocus();
    doc = new DocLnk(nf);
    updateCaption();
}

void TextEdit::openFile( const QString &f )
{
    bFromDocView = TRUE;
    DocLnk nf;
    nf.setType("text/plain");
    nf.setFile(f);
    openFile(nf);
    showEditTools();
}

void TextEdit::openFile( const DocLnk &f )
{
    clear();
    FileManager fm;
    QString txt;
    if ( !fm.loadFile( f, txt ) ) {
	// ####### could be a new file
	//qDebug( "Cannot open file" );
	//return;
    }
    fileNew();
    doc = new DocLnk(f);
    editor->setText(txt);
    editor->setEdited(FALSE);
    updateCaption();
}

void TextEdit::showEditTools()
{
    if ( !doc )
	close();
    fileSelector->hide();
    menu->show();
    editBar->show();
    if ( searchVisible )
	searchBar->show();
    updateCaption();
}

void TextEdit::save()
{
    if ( !doc )
	return;
    if ( !editor->edited() )
	return;

    QString rt = editor->text();
    QString pt = rt;

    if ( doc->name().isEmpty() ) {
	unsigned ispace = pt.find( ' ' );
	unsigned ienter = pt.find( '\n' );
	int i = (ispace < ienter) ? ispace : ienter;
	QString docname;
	if ( i == -1 ) {
	    if ( pt.isEmpty() )
		docname = "Empty Text";
	    else
		docname = pt;
	} else {
	    docname = pt.left( i );
	}
	doc->setName(docname);
    }
    FileManager fm;
    fm.saveFile( *doc, rt );
}

void TextEdit::clear()
{
    delete doc;
    doc = 0;
    editor->clear();
}

void TextEdit::updateCaption()
{
    if ( !doc )
	setCaption( tr("Text Editor") );
    else {
	QString s = doc->name();
	if ( s.isEmpty() )
	    s = tr( "Unnamed" );
	setCaption( s + " - " + tr("Text Editor") );
    }
}

void TextEdit::setDocument(const QString& fileref)
{
    bFromDocView = TRUE;
    openFile(DocLnk(fileref));
    showEditTools();
}

void TextEdit::closeEvent( QCloseEvent *e )
{
    if ( editorStack->visibleWidget() == editor && !bFromDocView ) {
	fileOpen();
	e->ignore();
    } else {
	bFromDocView = FALSE;
	e->accept();
    }
}
	
