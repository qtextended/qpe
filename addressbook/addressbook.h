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
#ifndef Addressbook_H
#define Addressbook_H

#include <qmainwindow.h>

class AbEditor;
class AbLabel;
class AbTable;
class AbEntry;
class AbEntryList;
class QToolBar;
class QToolButton;
class QDialog;

class AddressbookWindow: public QMainWindow
{
    Q_OBJECT
public:
    AddressbookWindow( QWidget *parent = 0, const char *name = 0, WFlags f = 0 );
    ~AddressbookWindow();

protected:
    void resizeEvent( QResizeEvent * e );
    void showList();
    void showView();
    enum EntryMode { NewEntry=0, EditEntry };
    void editEntry( EntryMode );
    void closeEvent( QCloseEvent *e );

private slots:
    void slotListNew();
    void slotListView();
    void slotListDelete();
    void slotViewBack();
    void slotViewEdit();
    void listIsEmpty( bool );
    void appMessage(const QCString& message, const QByteArray& data);
    void syncMessage( const QCString &msg, const QByteArray &data );
    void slotSettings();

private:
    void initFields();	// inititialize our fields...
    QToolBar *listTools, *viewTools;
    QToolButton *deleteButton;
    enum Panes { paneList=0, paneView, paneEdit };
    AbEditor *abEditor;
    AbLabel *abView;
    AbTable *abList;
    AbEntry *entry;
    AbEntryList *cel;
    bool bAbEditFirstTime;
    int viewMargin;
};

#endif
