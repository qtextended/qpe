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
#ifndef TODOTABLE_H
#define TODOTABLE_H

#include <qtable.h>
#include <qmap.h>

#include <organizer.h>
#include <qguardedptr.h>

class Node;
class QComboBox;
class QTimer;

class CheckItem : public QTableItem
{
public:
    CheckItem( QTable *t );

    void setChecked( bool b );
    void toggle();
    bool isChecked() const;
    QString key() const;

    void paint( QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected );

private:
    bool checked;

};

class ComboItem : public QTableItem
{
public:
    ComboItem( QTable *t, EditType et );
    QWidget *createEditor() const;
    void setContentFromEditor( QWidget *w );
    void setText( const QString &s );
    int alignment() const { return Qt::AlignCenter; }

    QString text() const;

private:
    QGuardedPtr<QComboBox> cb;

};

class TodoTable : public QTable
{
    Q_OBJECT

public:
    TodoTable( QWidget *parent = 0, const char * name = 0 );
    void addEntry( Todo *todo );

    Todo *currentEntry() const;
    void replaceEntry( Todo *todo );

    QStringList categories() const { return categoryList; }
    void addCategory( const QString &c ) { categoryList.append( c ); }

    void setShowCompleted( bool sc ) { showComp = sc; updateVisible(); }
    bool showCompleted() const { return showComp; }

    void setShowCategory( const QString &c ) { showCat = c; updateVisible(); }
    const QString &showCategory() const { return showCat; }

    void save( const QString &fn );
    void load( const QString &fn );
    void loadOldFormat( const QString &fn );
    void saveCategories( const QString &fn );
    void loadCategories( const QString &fn );
    void clear();

    void setPaintingEnabled( bool e );

    virtual void sortColumn( int col, bool ascending, bool /*wholeRows*/ );

signals:
    void signalEdit();
    void signalDoneChanged( bool b );
    void signalPriorityChanged( int i );
    void signalShowMenu( const QPoint & );

private:
    void updateVisible();
    void viewportPaintEvent( QPaintEvent * );

private slots:
    void slotClicked( int row, int col, int button, const QPoint &pos );
    void slotPressed( int row, int col, int button, const QPoint &pos );
    void slotCheckPriority(int row, int col );
    void slotCurrentChanged(int row, int col );
    void slotShowMenu();

private:
    QMap<CheckItem*, Todo*> todoList;
    QStringList categoryList;
    bool showComp;
    QString showCat;
    QTimer *menuTimer;
    bool enablePainting;
};

#endif
