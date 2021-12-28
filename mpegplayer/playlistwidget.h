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
#ifndef PLAY_LIST_WIDGET_H
#define PLAY_LIST_WIDGET_H


#include <qwidget.h>
#include <qlist.h>
#include <qlistview.h>
#include <qmainwindow.h>
#include <applnk.h>


class Config;
class PlayListWidgetPrivate;


class PlayListWidget : public QWidget {
    Q_OBJECT
public:
    PlayListWidget( QWidget* parent=0, const char* name=0, WFlags fl=0 );
    ~PlayListWidget();

    // retrieve the current playlist entry (media file link)
    const DocLnk *current();
    void setDocument(const QString& fileref);
    void readConfig( Config& );
    void writeConfig( Config& );

public slots:
    void addToSelection( const DocLnk & ); // Add a media file to the playlist
    void setShuffle( bool ); // Shuffle/randomize the playlist
    void setLooping( bool ); // Loop the playlist
    void setFullScreenSlot( bool ); // Display in Fullscreen
    void setScaledSlot( bool ); // Display one to one
    void showPlayList( bool ); // Show/Hide the playlist
    void save();  // Save the playlist
    void close(); // Close the playlist/open selector
    bool first();
    bool last();
    bool next();
    bool prev();

signals:
    void playClicked(); // Begin playing the playlist from the current entry
    void stopClicked(); // Stop playing the current entry
    void prevClicked(); // Play the previous entry
    void nextClicked(); // Play the next entry
    void setFullScreen( bool ); // Display in Fullscreen
    void setScaled( bool ); // Display one to one

private:
    void initializeStates();
    PlayListWidgetPrivate *d; // Private implementation data
};


class PlayListSelection : public QListView {
    Q_OBJECT
public:
    PlayListSelection( QWidget *parent, const char *name=0 );
    ~PlayListSelection();

    const DocLnk *current(); // retrieve the current playlist entry (media file link)

public slots:
    void addToSelection( const DocLnk & ); // Add a media file to the playlist
    void removeSelected();   // Remove a media file from the playlist
    void moveSelectedUp();   // Move the media file up the playlist so it is played earlier
    void moveSelectedDown(); // Move the media file down the playlist so it is played later
    bool prev();
    bool next();
    bool first();
    bool last();

protected:
    virtual void contentsMouseMoveEvent(QMouseEvent *);

private:
    QList<DocLnk>   selectedList;
    const DocLnk    *lnk;
};


#endif // PLAY_LIST_WIDGET_H
