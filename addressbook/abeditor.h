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
#ifndef ABEDITOR_H
#define ABEDITOR_H

#include <qdialog.h>
#include <qlist.h>
#include <qmap.h>
#include <qstringlist.h>


class AbEntry;
class AbEditorPage1;
class AbEditorPage2;
class QScrollView;
class QWidgetStack;
class QMultiLineEdit;

class AbEditor : public QDialog
{
    Q_OBJECT
public:
    AbEditor( AbEntry* entry, QWidget* parent = 0,
	      const char* name = 0, WFlags fl = 0 );
    ~AbEditor();
    void loadFields();

public slots:
    void slotPage1();
    void slotPage2();
    void slotNote();
    void setEntry( AbEntry* );
    void parseName();

protected:
    void resizeEvent( QResizeEvent *e );
protected slots:
    void accept();

private:
    void init();
    void initMap();
    void saveEntry();
    bool isEmpty();
    QDialog *dlgNote;
    QMultiLineEdit *txtNote;
    AbEntry *ent;
    QWidgetStack *staEditor;
    QScrollView *svPage;
    AbEditorPage1 *frmPage1;
    AbEditorPage2 *frmPage2;
    QMap<QString, QString> mapField;
    QStringList slFields,
                slValues;
};

#endif
