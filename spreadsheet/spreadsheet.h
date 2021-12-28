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
#ifndef SPREADSHEET_H
#define SPREADSHEET_H

#include "applnk.h"
#include <qapplication.h>
#include <qtable.h>
#include <qstring.h>
#include <qlineedit.h>
#include <qvaluelist.h>
#include <qinputdialog.h>
#include <qvaluestack.h>

// spreadsheet cell base class
class SpreadsheetCell : public QTableItem
{
 public:
    SpreadsheetCell ( const QString& t, QTable * table, EditType et = OnTyping )
	: QTableItem(table, et, QString::null)
    {
	setText( t );
    }
};

typedef QValueStack< double > DoubleStack;
typedef QValueStack< QChar > CharStack;

class EquationCell : public SpreadsheetCell
{
 public:
    EquationCell ( const QString& e, QTable * table )
	: SpreadsheetCell( QString::null, table, Never),
	  eq(e)
    {}
    QString equation() { return eq; }
    QString text() const { return eval( toPostfix( eq.simplifyWhiteSpace() ) ); }
protected:
    QString evalToken( QString& tkn ) const;
    int precedence( QChar c ) const;
    QString toPostfix( QString s ) const;
    QString eval( QString s ) const;
    void paint ( QPainter * p, const QColorGroup & cg, const QRect & cr, bool selected );
    QWidget * createEditor () const { return 0; }
private:
    QString eq;
};


class Spreadsheet : public QTable
{
    Q_OBJECT
public:
    Spreadsheet ( QWidget * parent = 0, const char * name = 0 );
    ~Spreadsheet(){}

    void setEditor( QLineEdit* le );
protected:
    void createTable();
public slots:
    void newFile( const DocLnk & );
    void openFile( const DocLnk & );
    void closeFile();
    void save();
    void clearCurrent();
    virtual void editEquation();
    virtual void newSelection( int row, int col );
signals:
    void newTextSelected( const QString& text);
protected:
    void setCellContentFromEditor ( int row, int col );
    bool eventFilter( QObject* o, QEvent* e );
protected slots:
    void textSelected( const QString& text );
private:
    DocLnk currentFile;
    QLineEdit* editor;
    bool editingFile;
    void clear();
    int itemCount();
};
#endif
