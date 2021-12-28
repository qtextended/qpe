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
#include <qaction.h>
#include <qlayout.h>
#include <qtoolbutton.h>
#include <qpushbutton.h>
#include <qheader.h>
#include <qmainwindow.h>
#include <qmessagebox.h>
#include <qlistbox.h>
#include <fileselector.h>
#include <applnk.h>
#include <resource.h>
#include <qlist.h>
#include <stdlib.h>
#include "playlistwidget.h"
#include <config.h>


class PlayListSelectionItem : public QListViewItem {
public:
    PlayListSelectionItem( QListView *parent, const DocLnk *f ) : QListViewItem( parent ), fl( f ) {
	setText( 0, f->name() );
	setPixmap( 0, f->pixmap() );
    }

    ~PlayListSelectionItem() {
    };

    const DocLnk *file() const { return fl; }

private:
    const DocLnk *fl;
};


PlayListSelection::PlayListSelection( QWidget *parent, const char *name )
    : QListView( parent, name )
{
    setAllColumnsShowFocus( TRUE );
    addColumn( tr( "Playlist Selection" ) );
    header()->hide();
    setSorting( -1, FALSE );
}


PlayListSelection::~PlayListSelection() {
}


void PlayListSelection::contentsMouseMoveEvent( QMouseEvent *event ) {
    if ( event->state() == QMouseEvent::LeftButton ) {
	QListViewItem *currentItem = selectedItem();
	QListViewItem *itemUnder = itemAt( event->pos() );
	if ( currentItem && currentItem->itemAbove() == itemUnder )
	    moveSelectedUp();
	else if ( currentItem && currentItem->itemBelow() == itemUnder )
	    moveSelectedDown();
    }
}


const DocLnk *PlayListSelection::current() {
    PlayListSelectionItem *item = (PlayListSelectionItem *)selectedItem();
    if ( item )
	return item->file();
    return NULL;
}


void PlayListSelection::addToSelection( const DocLnk &lnk ) {
    PlayListSelectionItem *item = new PlayListSelectionItem( this, new DocLnk( lnk ) );
    QListViewItem *current = selectedItem();
    if ( current )
        item->moveItem( current );
    setSelected( item, TRUE );
    ensureItemVisible( selectedItem() );
}


void PlayListSelection::removeSelected() {
    QListViewItem *item = selectedItem();
    if ( item )
	delete item;
    setSelected( currentItem(), TRUE );
    ensureItemVisible( selectedItem() );
}


void PlayListSelection::moveSelectedUp() {
    QListViewItem *item = selectedItem();
    if ( item && item->itemAbove() )
	item->itemAbove()->moveItem( item );
    ensureItemVisible( selectedItem() );
}


void PlayListSelection::moveSelectedDown() {
    QListViewItem *item = selectedItem();
    if ( item && item->itemBelow() )
        item->moveItem( item->itemBelow() );
    ensureItemVisible( selectedItem() );
}


bool PlayListSelection::prev() {
    QListViewItem *item = selectedItem();
    if ( item && item->itemAbove() )
        setSelected( item->itemAbove(), TRUE );
    else
	return FALSE;
    ensureItemVisible( selectedItem() );
    return TRUE;
}


bool PlayListSelection::next() {
    QListViewItem *item = selectedItem();
    if ( item && item->itemBelow() )
        setSelected( item->itemBelow(), TRUE );
    else
	return FALSE;
    ensureItemVisible( selectedItem() );
    return TRUE;
}


bool PlayListSelection::first() {
    QListViewItem *item = firstChild();
    if ( item )
        setSelected( item, TRUE );
    else
	return FALSE;
    ensureItemVisible( selectedItem() );
    return TRUE;
}


bool PlayListSelection::last() {
    QListViewItem *prevItem = NULL;
    QListViewItem *item = firstChild();
    while ( ( item = item->nextSibling() ) )
	prevItem = item;
    if ( prevItem )
        setSelected( prevItem, TRUE );
    else
	return FALSE;
    ensureItemVisible( selectedItem() );
    return TRUE;
}


class PlayListWidgetPrivate {
public:
    bool isLooping;
    bool isShuffled;
    bool isFullScreen;
    bool isScaled;
    bool usePlayList;

    QPushButton *shuffle;
    QPushButton *cut;
    QPushButton *fullscreen;
    QPushButton *loop;
    QPushButton *scale;
    QPushButton *playlist;

    QFrame *playListFrame;
    FileSelector *files;
    PlayListSelection *selectedFiles;
    bool setDocumentUsed;
    DocLnk *current;
};


QPushButton *addButton( const QString &name, const QString &icon, QWidget *parent, QBoxLayout *layout, int stretch, bool isToggle, QWidget *handler, const QString &slot ) {
    QPushButton *button = new QPushButton( name, parent );
    if ( icon )
	button->setPixmap( Resource::loadPixmap( icon ) );
    button->setFlat( TRUE );
    button->setToggleButton( isToggle );
    button->connect( button, isToggle ? SIGNAL( toggled(bool) ) : SIGNAL( pressed() ), handler, slot );
    layout->addWidget( button, stretch );
    return button;
}


PlayListWidget::PlayListWidget( QWidget* parent, const char* name, WFlags fl ) : QWidget( parent, name, fl ) {
    d = new PlayListWidgetPrivate;
    d->isLooping = FALSE;
    d->isShuffled = FALSE;
    d->isFullScreen = FALSE;
    d->isScaled = FALSE;
    d->usePlayList = FALSE;
    d->setDocumentUsed = FALSE;
    d->current = NULL;

    setBackgroundMode( PaletteButton );

    QVBoxLayout *vbox = new QVBoxLayout( this );
    QHBoxLayout *toolbar = new QHBoxLayout( vbox );

    QVBox *vbox2 = new QVBox( this );
    d->playListFrame = new QFrame( vbox2 );
    d->playListFrame->setBackgroundMode( PaletteButton );
    QVBoxLayout *selectedArea = new QVBoxLayout( d->playListFrame );

    d->files = new FileSelector( "*/mpeg;audio/x-wav", vbox2, "Find Media Files", FALSE, FALSE );//, TRUE );
    d->selectedFiles = new PlayListSelection( d->playListFrame );

    QFrame *x = new QFrame( d->playListFrame );
    x->setBackgroundMode( PaletteButton );
//    QHBoxLayout *buttons = new QHBoxLayout( x );

    selectedArea->addWidget( x, 0 );
    selectedArea->addWidget( d->selectedFiles, 1 );

//  addButton(  "Rotation",      "rotate", this, toolbar, 0,  TRUE,             this, SLOT( setLooping(bool) ) );
    d->fullscreen = addButton("FullScreen",  "fullscreen", this, toolbar, 0,  TRUE,             this, SLOT( setFullScreenSlot(bool) ) );
    d->scale = addButton(     "Scale", "mpegplayer/scale", this, toolbar, 0,  TRUE,             this, SLOT( setScaledSlot(bool) ) );
    d->loop = addButton(      "Loop", "mpegplayer/loop", this, toolbar, 0,  TRUE,             this, SLOT( setLooping(bool) ) );
    addButton(     "Start",        "play", this, toolbar, 0, FALSE,             this, SIGNAL( playClicked() ) );
    addButton(      "Stop",        "stop", this, toolbar, 0, FALSE,             this, SIGNAL( stopClicked() ) );
//  addButton(      "Save",        "save", this, toolbar, 1, FALSE,             this, SLOT( save() ) );
//  addButton(     "Close",       "close", this, toolbar, 1, FALSE,             this, SLOT( close() ) );
    d->playlist = addButton(  "PlayList",          NULL, this, toolbar, 0,  TRUE,             this, SLOT( showPlayList(bool) ) );

    d->shuffle = addButton(   "Shuffle", "mpegplayer/shuffle",  this, toolbar, 0,  TRUE,             this, SLOT( setShuffle(bool) ) );
//    addButton(  "Previous",        "back",    x, buttons, 0, FALSE,		this, SIGNAL( prevClicked() ) );
    d->cut = addButton(    "Delete",         "cut",    this, toolbar, 0, FALSE, d->selectedFiles, SLOT( removeSelected() ) );
//    addButton(      "Next",     "forward",    x, buttons, 0, FALSE,		this, SIGNAL( nextClicked() ) );

    // connect( files, SIGNAL( fileSelected( const DocLnk & ) ), d->selectedFiles, SLOT( addToSelection( const DocLnk & ) ) );
    connect( d->files, SIGNAL( fileSelected( const DocLnk & ) ), this, SLOT( addToSelection( const DocLnk & ) ) );

    showPlayList( d->usePlayList );

    vbox->addWidget( vbox2, 1 );

    initializeStates();
}


PlayListWidget::~PlayListWidget() {
    delete d;
}


void PlayListWidget::initializeStates() {
    if ( d->isScaled )
	d->scale->toggle();
    if ( d->isFullScreen )
	d->fullscreen->toggle();
    if ( d->isLooping )
	d->loop->toggle();
    if ( d->isShuffled )
	d->shuffle->toggle();
    if ( d->usePlayList )
	d->playlist->toggle();

    setFullScreenSlot( d->isFullScreen );
    setScaledSlot( d->isScaled );
    showPlayList( d->usePlayList );
    setLooping( d->isLooping );
    setShuffle( d->isShuffled );
}


void PlayListWidget::readConfig( Config& cfg ) {
    cfg.setGroup("Options");
    d->isFullScreen = cfg.readBoolEntry( "FullScreen" );
    d->isScaled = cfg.readBoolEntry( "Scale" );
    d->isLooping = cfg.readBoolEntry( "Looping" );
    d->isShuffled = cfg.readBoolEntry( "Shuffle" );
    d->usePlayList = cfg.readBoolEntry( "UsePlayList" );

    initializeStates();

    cfg.setGroup("PlayList");
    int noOfFiles = cfg.readNumEntry("NumberOfFiles", 0 );
    for ( int i = 0; i < noOfFiles; i++ ) {
	QString entryName;
	entryName.sprintf( "File%i", i + 1 );
	QString linkFile = cfg.readEntry( entryName );
	DocLnk lnk( linkFile );
	if ( lnk.isValid() )
            d->selectedFiles->addToSelection( lnk ); 
    }
}


void PlayListWidget::writeConfig( Config& cfg ) {
    cfg.setGroup("Options");
    cfg.writeEntry("FullScreen", d->isFullScreen );
    cfg.writeEntry("Scale", d->isScaled );
    cfg.writeEntry("Looping", d->isLooping );
    cfg.writeEntry("Shuffle", d->isShuffled );
    cfg.writeEntry("UsePlayList", d->usePlayList );

    cfg.setGroup("PlayList");

    int noOfFiles = 0;

    d->selectedFiles->first();
    do {
	const DocLnk *lnk = d->selectedFiles->current();
	if ( lnk ) {
	    QString entryName;
	    entryName.sprintf( "File%i", noOfFiles + 1 );
	    cfg.writeEntry( entryName, lnk->linkFile() );
	    noOfFiles++;
	}
    } while ( d->selectedFiles->next() );

    cfg.writeEntry("NumberOfFiles", noOfFiles );
}


void PlayListWidget::addToSelection( const DocLnk& lnk ) {
    if ( d->usePlayList )
        d->selectedFiles->addToSelection( lnk ); 
    else
	emit playClicked();
}


void PlayListWidget::setDocument(const QString& fileref)
{
    if ( d->usePlayList )
	addToSelection( DocLnk( fileref ) );
    else {
	d->setDocumentUsed = TRUE;
	d->current = new DocLnk( fileref );
    }
}


const DocLnk *PlayListWidget::current() {
    if ( d->usePlayList )
	return d->selectedFiles->current();
    else if ( d->setDocumentUsed && d->current ) {
	d->setDocumentUsed = FALSE;
	return d->current;
    } else
	return d->files->selected();
}


bool PlayListWidget::prev() {
    if ( d->usePlayList ) {
	if ( d->isShuffled ) {
	    int j = 1 + (int)(97.0 * rand() / (RAND_MAX + 1.0));
	    for ( int i = 0; i < j; i++ ) {
		if ( !d->selectedFiles->next() )
		    d->selectedFiles->first();
	    }
	    return TRUE;
	} else {
	    if ( !d->selectedFiles->prev() ) {
		if ( d->isLooping ) {
		    return d->selectedFiles->last();
		} else {
		    return FALSE;
		}
	    }
	    return TRUE;
	}
    } else {
	return d->isLooping;
    }
}


bool PlayListWidget::next() {
    if ( d->usePlayList ) {
	if ( d->isShuffled ) {
	    return prev();
	} else {
	    if ( !d->selectedFiles->next() ) {
		if ( d->isLooping ) {
		    return d->selectedFiles->first();
		} else {
		    return FALSE;
		}
	    }
	    return TRUE;
	}
    } else {
	return d->isLooping;
    }
}


bool PlayListWidget::first() {
    if ( d->usePlayList )
	return d->selectedFiles->first();
    else
	return d->isLooping;
}


bool PlayListWidget::last() {
    if ( d->usePlayList )
	return d->selectedFiles->last();
    else
	return d->isLooping;
}


void PlayListWidget::save() {
}


void PlayListWidget::setFullScreenSlot( bool full ) {
    d->isFullScreen = full;
    emit setFullScreen( full );
}


void PlayListWidget::setScaledSlot( bool scale ) {
    d->isScaled = scale;
    emit setScaled( scale );
}


void PlayListWidget::setShuffle( bool shuffle ) {
    d->isShuffled = shuffle;
}


void PlayListWidget::setLooping( bool loop ) {
    d->isLooping = loop;
}


void PlayListWidget::showPlayList( bool showit ) {
    d->usePlayList = showit;
    if ( d->usePlayList ) {
	d->playListFrame->show();
	d->shuffle->show();
	d->cut->show();
    } else {
	d->playListFrame->hide();
	d->shuffle->hide();
	d->cut->hide();
    }
}


void PlayListWidget::close() {
}

