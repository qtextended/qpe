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
#ifndef PIMSPREADSHEETWINDOW_H
#define PIMSPREADSHEETWINDOW_H
#include "spreadsheetwidget.h"
#include "applnk.h"
#include <qmainwindow.h>

class FileSelector;
class QAction;
class QToolbar;
class QLineEdit;
class PIMSpreadsheetWindow : public QMainWindow
{
    Q_OBJECT
public:
    PIMSpreadsheetWindow ( QWidget * parent = 0, const char * name = 0, WFlags=0 );
    ~PIMSpreadsheetWindow();
public slots:
    void fileNew();
    void fileOpen();
    void fileClose();
    void newFile( const DocLnk & );
    void openFile( const DocLnk & );
    void setDocument(const QString&);
private slots:
    void closeFileSelector();
private:
    FileSelector* fileSelector;
    DocLnk currentFile;
    QToolBar *menu, *editBar;
    QAction *editEquationAction, *clearCellAction;
    SpreadsheetWidget* spreadsheet;
};

#endif
