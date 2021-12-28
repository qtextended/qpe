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
#include "abtable.h"
#include "abentry.h"

#include <config.h>

/*!
  \class AbTableItem abtable.h

  \brief QTableItem based class for showing a field of an entry
*/

AbTableItem::AbTableItem( QTable *t, EditType et, const QString &s,
			  const QString &k, int alignment )
    : QTableItem( t, et, s ), align( alignment )
{
    entKey = k;
}

QString AbTableItem::entryKey() const
{
    return entKey;
}

void AbTableItem::setEntryKey( const QString & k )
{
    entKey = k;
}

int AbTableItem::alignment() const
{
    return align;
}

QString AbTableItem::key() const
{
    return text().lower();
}

/*!
  \class AbTable abtable.h

  \brief QTable based class for showing a list of entries
*/

AbTable::AbTable( QWidget *parent, const char *name )
    : QTable( parent, name ), lastSortCol( -1 ), asc( TRUE )
{
    setSorting( TRUE ); //TODO get sorting working
    setSelectionMode( NoSelection );
}

AbTable::~AbTable()
{
}

void AbTable::init( AbEntryList *iCel )
{
    //qDebug( "in init" );
    // init the map...
    Config cfg1( "AddressBook" );
    Config cfg2( "AddressBook" );
    QString strCfg1,
	    strCfg2;
    int i;

    // This stuff better exist...
    cfg1.setGroup( "AddressFields" );
    cfg2.setGroup( "XMLFields" );
    i = 0;
    strCfg1 = cfg1.readEntry( "Field" + QString::number(i), QString::null );
    strCfg2 = cfg2.readEntry( "XMLField" + QString::number(i++),
			      QString::null );
    while ( !strCfg1.isNull() && !strCfg2.isNull() ) {
	mapFields.insert( strCfg1, strCfg2 );
	strCfg1 = cfg1.readEntry( "Field" + QString::number(i),
				  QString::null );
	strCfg2 = cfg2.readEntry( "XMLField" + QString::number(i++),
				  QString::null );
    }

    loadFields();

    QDictIterator<AbEntry> it( *iCel );
    cel = iCel;

    setNumRows( 0 );
    setNumCols( 2 );

    horizontalHeader()->setLabel( 0, tr( "Full Name" ));
    horizontalHeader()->setLabel( 1, tr( "Contact" ));
    setLeftMargin( 0 );
    verticalHeader()->hide();
    
    //qDebug( "start iterating" );
    QDictIterator<AbEntry> jt( *cel );
    if ( !jt.current() )
	emit empty( TRUE );
    setNumRows( jt.count() );
    while ( jt.current() ) {
        insertEntry( jt.current(), jt.currentKey() );
        ++jt;
    }
    //qDebug( "leaving init" );
}

void AbTable::setCell( int row, int col, const QString &value,
		       const QString &key )
{
    int align = AlignLeft | AlignVCenter;
    if (col == 1)
	align = AlignRight | AlignVCenter;
    QTableItem *ati;
    delete item( row, col );
    ati = new AbTableItem( this, QTableItem::WhenCurrent, value, key, align );
    setItem( row, col, ati );
}

void AbTable::setRow( int row, AbEntry *entry, QString key )
{
    QString firstName, lastName, name;
    QString fieldNum, indicator;
    const QString *value;

    value = entry->find( "LastName" );
    if ( value )
	lastName = *value;
    value = entry->find( "FirstName" );
    if ( value )
	firstName = *value;
    if (!lastName.isEmpty() && !firstName.isEmpty())
	name = " " + lastName + ", " + firstName;
    else {
	// try to do a bit better, maybe the compnay...
	value = entry->find( "Company" );
	if ( value && !value->isEmpty() )
	    name = " " + *value;
	else
	    name = " " + firstName + lastName;
    }
    setCell( row, 0, name, key );

    // Use the Magic of Entries to get different contacts, depending
    // on what someone wants...
    int t;
    int slCount = slFields.count();
    for ( t = 0; t < slCount; t++ ) {
	value = entry->find( mapFields[slFields[t]] );
	if ( value ) {
	    setCell( row, 1, *value, key );
	    break;
	} else
	    setCell( row, 1, "", key );
    }
}

void AbTable::insertEntry( AbEntry *entry, const QString &key )
{
    // Now I am fast, but I assume I'm being called from inside...
    static int i = 0;
    if ( i == numRows() )
	i = 0;
    setRow( i++, entry, key );
    if ( i == 0)
	emit empty( FALSE );
}

void AbTable::appendEntry( AbEntry *entry, const QString &key )
{
    int i = numRows();
    setNumRows( i + 1 );
    setRow( i, entry, key );
    if (i == 0)
	emit empty( FALSE );
}

void AbTable::columnClicked( int col )
{
    if ( !sorting() )
	return;

    if ( col == lastSortCol ) {
	asc = !asc;
    } else {
	lastSortCol = col;
	asc = TRUE;
    }

    sortColumn( lastSortCol, asc, TRUE );
}

AbEntry* AbTable::currentEntry()
{
    AbTableItem *abItem = (AbTableItem*)item( currentRow(), 0 );
    if (!abItem)
	return 0;

    QString key = abItem->entryKey();
    return cel->find( key );
}

void AbTable::replaceCurrentEntry( AbEntry* newEntry )
{
    AbEntry *entry = currentEntry();
    ASSERT(entry);

    endEdit( currentRow(), currentColumn(), FALSE, FALSE );
    QTableItem *ti = item( currentRow(), 0 );
    ASSERT( ti );
    AbTableItem *ati = static_cast< AbTableItem* >(ti);
    QString key = ati->entryKey();
    cel->replace( key, newEntry );
    setRow( currentRow(), newEntry, key );
}

void AbTable::deleteCurrentEntry()
{
    endEdit( currentRow(), currentColumn(), FALSE, FALSE );
    if ( currentRow() >= 0 ) {
	AbTableItem *abItem = (AbTableItem*)item( currentRow(), 0 );
	if (!abItem)
	    return;
	QString key = abItem->entryKey();
        cel->remove( key );
        for ( int i = currentRow(); i < numRows() - 1; ++i ) {
	    abItem = (AbTableItem*)item( i + 1, 0 );
	    key = abItem->entryKey();
	    AbEntry *entry = cel->find( key );
	    ASSERT( entry );
	    setRow( i, entry, key );
        }
        setNumRows( numRows() - 1 ); //QGVector::remove: Index 4 out of range
    }

    if ( numRows() == 0 )
	emit empty( TRUE );
}

QWidget *AbTable::createEditor( int, int, bool ) const
{
    return 0;
}

void AbTable::clear()
{
    cel->clear();
    for ( int r = 0; r < numRows(); ++r ) {
	for ( int c = 0; c < numCols(); ++c ) {
            if ( cellWidget( r, c ) )
                clearCellWidget( r, c );
	    clearCell( r, c );
	}
    }
    setNumRows( 0 );
}

void AbTable::loadFields()
{
    slFields.clear();
    Config cfg( "AddressBook" );
    cfg.setGroup( "ImportantCategory" );

    int i = 0;
    QString str;
    str = cfg.readEntry( "Category" + QString::number(i++), QString::null );
    while ( !str.isNull() ) {
	slFields.append(str);
	str = cfg.readEntry( "Category" + QString::number(i++), QString::null );
    }
}

void AbTable::refresh()
{
    for ( int r = 0; r < numRows(); ++r ) {
	for ( int c = 0; c < numCols(); ++c ) {
            if ( cellWidget( r, c ) )
                clearCellWidget( r, c );
	    clearCell( r, c );
	}
    }
    setNumRows( 0 );
    QDictIterator<AbEntry> jt( *cel );
    if ( !jt.current() )
	emit empty( TRUE );
    setNumRows( jt.count() );
    while ( jt.current() ) {
        insertEntry( jt.current(), jt.currentKey() );
        ++jt;
    }
}


#if QT_VERSION <= 230
#ifndef SINGLE_APP
void QTable::paintEmptyArea( QPainter *p, int cx, int cy, int cw, int ch )
{
    // Region of the rect we should draw
    QRegion reg( QRect( cx, cy, cw, ch ) );
    // Subtract the table from it
    reg = reg.subtract( QRect( QPoint( 0, 0 ), tableSize() ) );
    // And draw the rectangles (transformed as needed)
    QArray<QRect> r = reg.rects();
    for (unsigned int i=0; i<r.count(); i++)
        p->fillRect( r[i], colorGroup().brush( QColorGroup::Base ) );
}
#endif
#endif
