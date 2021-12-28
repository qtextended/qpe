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
#ifndef ABENTRY_H
#define ABENTRY_H

#include <qobject.h>
#include <qdict.h>
#include <qstring.h>
#include <qtextstream.h>
#include <qlist.h>
#include <qstringlist.h>

/**
 * The AbEntry class is used to store the current state of an address
 * book entry being edited. It is used in conjunction with AbEditor,
 * and in doing so it plays the Document part of the View/Document pattern.
 *
 * An instance of this call is also an Observer emitting the changed() signal
 * to inform other objects that it has been edited. This allows different
 * widgets showing the same entry field in the corresponding AbEditor
 * object to be kept synchronized.
 *
 * This class is just a placeholder until it can be replaced with something
 * more suitable. It is incomplete, for instance no complementary
 * ContactEntryIterator and ContactEntryList classes has been defined,
 * ad hoc load and save methods exist that will have to be removed.
 *
 * A couple of conventions are used
 * Fields beginning with the prefix "." won't be saved.
 * Fields beginning with the prefix "X-CUSTOM-" are custom fields.
 *
 * The underlying implementation is based on a QDict<QString> object.
 **/

class AbEntry;
class Node;

typedef QDictIterator<AbEntry> AbEntryListIterator;

class AbEntryList : public QDict<AbEntry>
{
public:
    AbEntryList();
    AbEntryList( const QString &filename );
    QString key();
    QString insert( AbEntry *item );
    void unremove( const QString &key, AbEntry *item );
    void save( const QString & filename );
    void load( const QString & filename );

    void saveXML( const QString & filename );
    void loadXML( const QString & filename );

protected:
    long kkey;
    QList<AbEntry> list;
};

class AbEntry
{

public:
/**
 * Creates a new AbEntry object.
 */
    AbEntry();
    AbEntry( const AbEntry &r );
    AbEntry& operator=( const AbEntry &r );

/**
 * Creates a AbEntry object from data stored in a textstream.
 *
 * Arguments:
 *
 * @param the name of the textstream.
 */
    AbEntry( QTextStream &t );

    AbEntry( Node *n );

/**
 * Returns a list of all custom fields, that is those beginning with
 * the prefix "X-CUSTOM-"
 */
    QStringList custom() const;

/**
 * Saves the entry to a text stream with the given filename.
 */
    void save( QTextStream &t );
    void saveXML( QTextStream &t );

/**
 * Loads the entry from a text stream with the given filename
 */
    void load( QTextStream &t );
    void loadXML( Node *n );

/**
 * Inserts a new key/value pair
 */
    void insert( const QString key, const QString *value );

/**
 * Updates the value associated with a key. The old value
 * will be deleted.
 */
    void replace( const QString key, const QString *value );

/**
 * Remove a key and deletes its associated value.
 */
    bool remove ( const QString key );

/**
 * Returns a const pointer to the value associated with a key.
 */
    const QString *find ( const QString & key ) const ;

/**
 * Returns a const pointer to the value associated with a key.
 */
    const QString *operator[] ( const QString & key ) const;

/**
 * Cause the changed signal to be emitted.
 */
    void touch();

/**
 * Remove all key/value pairs stored.
 */
    void clear();
    
    QDict<QString> dict; // This unfortunately doesn't make a good base class
  // It's not derived from QOBject and the majority of methods are not virtual
};

#endif
