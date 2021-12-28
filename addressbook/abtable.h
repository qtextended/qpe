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
#ifndef ABTABLE_H
#define ABTABLE_H

#include <qmap.h>
#include <qtable.h>
#include <qstringlist.h>

class AbEntryList;
class AbEntry;


class AbTableItem : public QTableItem
{
public:
    AbTableItem( QTable *t, EditType et, const QString &s, const QString &k,
		 int alignment );
    QString entryKey() const;
    void setEntryKey( const QString & k );
    virtual int alignment() const;
    virtual QString key() const;

private:
    QString entKey;
    int align;
};

class AbTable : public QTable
{
    Q_OBJECT

public:
    AbTable( QWidget *parent, const char *name=0 );
    ~AbTable();

    void init( AbEntryList* );

    void setCell( int row, int col, const QString &value, const QString &key );
    void setRow( int row, AbEntry *entry, QString key );
    void appendEntry( AbEntry *newEntry, const QString &key );
    AbEntry* currentEntry();
    void replaceCurrentEntry( AbEntry* );
    void deleteCurrentEntry();
    void clear();
    void loadFields();
    void refresh();

signals:
    void empty( bool );

protected:
    virtual QWidget *createEditor( int, int, bool ) const;

protected slots:
    virtual void columnClicked( int col );

private:
    void insertEntry( AbEntry *entry, const QString &key );
    AbEntryList *cel;
    QStringList slFields;
    int lastSortCol;
    bool asc;
    QMap<QString, QString> mapFields;
};
#endif // ABTABLE_H
