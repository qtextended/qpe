/**********************************************************************
** Copyright (C) 2001 Trolltech AS.  All rights reserved.
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
** $Id: addresssettings.cpp,v 1.6 2001/08/27 16:55:18 twschulz Exp $
**
**********************************************************************/


#include <qcombobox.h>
#include "addresssettings.h"
#include "config.h"
// make this is a const in case there is a need to change it
static const int MAX_ITEMS = 8;

static bool findDuplicates( int aIndex[], int value, int start,
			    int *where = NULL );


AddressSettings::AddressSettings( QWidget *parent, const char *name )
     : AddressSettingsBase( parent, name, TRUE )
{
    init();
}

AddressSettings::~AddressSettings()
{
}

QStringList AddressSettings::categories()
{
    int i;
    QStringList myList;
    QListIterator<QComboBox> itItems( listItems );
    for( i = 0; *itItems; ++itItems, i++ )
	myList[i] = itItems.current()->currentText();

    return myList;
}

void AddressSettings::init()
{
    Config cfg( "AddressBook" );

    QStringList slFields;
    int i;
    cfg.setGroup( "AddressFields" );
    i = 0;
    QString strField = cfg.readEntry( "Field" + QString::number(i++),
				      QString::null );
    while ( !strField.isNull() ) {
	slFields.append( strField );
	strField = cfg.readEntry( "Field" + QString::number(i++),
				  QString::null );
    }

    // first add in our combos
    listItems.setAutoDelete( FALSE );
    listItems.append( cmbImport1 );
    listItems.append( cmbImport2 );
    listItems.append( cmbImport3 );
    listItems.append( cmbImport4 );
    listItems.append( cmbImport5 );
    listItems.append( cmbImport6 );
    listItems.append( cmbImport7 );
    listItems.append( cmbImport8 );

    // we must populate our comboboxes...
    QListIterator<QComboBox> itItems( listItems );
    for ( ; *itItems; ++itItems ) {
	for ( i = 0; i < int(slFields.count()); i++ )
	    (*itItems)->insertItem( slFields[i] );
    }
    itItems.toFirst();
    cfg.setGroup( "ImportantCategory" );

    for ( i = 0; i < MAX_ITEMS; i++ ) {
	QString zn = cfg.readEntry( "Category" + QString::number(i),
				    QString::null );
	if ( zn.isNull() )
	    break;
	(*itItems)->setCurrentItem( slFields.findIndex(zn) );
	++itItems;
    }
}

void AddressSettings::slotMakeAllUnique()
{
    // we should make sure that all combo's contian unique selections
    QListIterator<QComboBox> itItems( listItems );
    int aIndex[MAX_ITEMS],
	iWhere, // where the duplicate is...
	k;


    for ( k = 0; *itItems && k < MAX_ITEMS; ++itItems, k++ ) {
	aIndex[k] = (*itItems)->currentItem();
    };

    // I'm sure this isn't the fastest but it should work,
    // it also only prefers things in the front of the list,
    // but I think that is okay...
    for ( k = 0; k < MAX_ITEMS; k++ ) {
	while ( findDuplicates(aIndex, aIndex[k], k + 1, &iWhere) ) {
	    int duplicate = aIndex[iWhere];
	    do {
		if ( duplicate == int(listItems.at(1)->count()) )
		    duplicate = -1;
		aIndex[iWhere] = ++duplicate;
	    } while ( findDuplicates( aIndex, aIndex[iWhere], iWhere + 1 ) );
	    (listItems.at(iWhere))->setCurrentItem( duplicate );
	}
    }
}

static bool findDuplicates( int aIndex[], int value, int start, int *where )
{
    // brute force
    int x;
    if ( where == NULL )
	x = 0;
    else
	x = start;
    for ( ; x < MAX_ITEMS; x++ ) {
	if ( aIndex[x] == value && x != start - 1 ) {
	    if ( where != NULL )
		*where = x;
	    return TRUE;
	}
    }
    return FALSE;
}

void AddressSettings::accept()
{
    save();
    QDialog::accept();
}


void AddressSettings::save()
{
    int i;
    Config cfg( "AddressBook" );
    cfg.setGroup( "ImportantCategory" );
    QListIterator<QComboBox> itItems( listItems );
    // write in all 8 entries
    for ( i = 0; *itItems; ++itItems, i++ )
	cfg.writeEntry( "Category" + QString::number(i),
			itItems.current()->currentText() );
}
