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
#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#define QTEXTEDIT_OPEN_API


#include <qmainwindow.h>
#include <qmultilineedit.h>
#include <qlist.h>
#include <filemanager.h>
#include <qmap.h>

class QWidgetStack;
class QToolButton;
class QPopupMenu;
class QToolBar;
class QLineEdit;
class QAction;
class FileSelector;

class TextEdit : public QMainWindow
{
    Q_OBJECT

public:
    TextEdit( QWidget *parent = 0, const char *name = 0, WFlags f = 0 );
    ~TextEdit();

    void openFile( const QString & );

protected:
    void closeEvent( QCloseEvent *e );

private slots:
    void setDocument(const QString&);

    void fileNew();
    void fileRevert();
    void fileOpen();

    void editCut();
    void editCopy();
    void editPaste();
    void editFind();

    void findNext();
    void findClose();

    void search();

    void newFile( const DocLnk & );
    void openFile( const DocLnk & );
    void showEditTools();

private:
    void colorChanged( const QColor &c );
    void save();
    void clear();
    void updateCaption();

private:
    QWidgetStack *editorStack;
    FileSelector *fileSelector;
    QMultiLineEdit* editor;
    QToolBar *menu, *editBar, *searchBar;
    QLineEdit *searchEdit;
    DocLnk *doc;
    bool searchVisible;
    bool bFromDocView;
};

#endif
