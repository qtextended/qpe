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
** $Id: abeditorpage2.cpp,v 1.4 2001/08/26 17:44:25 twschulz Exp $
**
**********************************************************************/

#include <qlabel.h>
#include <qlineedit.h>

#include <config.h>

#include "abeditorpage2.h"


AbEditorPage2::AbEditorPage2( QWidget *parent, const char *name )
    : AbEditorPage2Base( parent, name )
{
    init();
}

AbEditorPage2::~AbEditorPage2()
{
}


QStringList AbEditorPage2::fields()
{
    QListIterator<QLabel> it( listLabels );
    QStringList sl;
    for ( ; *it; ++it ) {
	sl.append( (*it)->text() );
    }
    return sl;
}

QStringList AbEditorPage2::values()
{
    QListIterator<QLineEdit> it( listTxt );
    QStringList sl;
    for ( ; *it; ++it ) {
	sl.append( (*it)->text() );
    }
    return sl;
}

void AbEditorPage2::init()
{
    // get the strings we need and set up the labels...
    listLabels.setAutoDelete( FALSE );
    listTxt.setAutoDelete( FALSE );
    listLabels.append( lblExtra1 );
    listLabels.append( lblExtra2 );
    listLabels.append( lblExtra3 );
    listLabels.append( lblExtra4 );
    listLabels.append( lblExtra5 );
    listLabels.append( lblExtra6 );
    listLabels.append( lblExtra7 );
    listLabels.append( lblExtra8 );
    listLabels.append( lblExtra9 );
    listLabels.append( lblExtra10 );
    listLabels.append( lblExtra11 );
    listLabels.append( lblExtra12 );
    listLabels.append( lblExtra13 );
    listLabels.append( lblExtra14 );
    listLabels.append( lblExtra15 );
    listLabels.append( lblExtra16 );
    listLabels.append( lblExtra17 );
    listLabels.append( lblExtra18 );
    listLabels.append( lblExtra19 );

    listTxt.append( txtExtra1 );
    listTxt.append( txtExtra2 );
    listTxt.append( txtExtra3 );
    listTxt.append( txtExtra4 );
    listTxt.append( txtExtra5 );
    listTxt.append( txtExtra6 );
    listTxt.append( txtExtra7 );
    listTxt.append( txtExtra8 );
    listTxt.append( txtExtra9 );
    listTxt.append( txtExtra10 );
    listTxt.append( txtExtra11 );
    listTxt.append( txtExtra12 );
    listTxt.append( txtExtra13 );
    listTxt.append( txtExtra14 );
    listTxt.append( txtExtra15 );
    listTxt.append( txtExtra16 );
    listTxt.append( txtExtra17 );
    listTxt.append( txtExtra18 );
    listTxt.append( txtExtra19 );

    loadFields();
}

void AbEditorPage2::slotHide()
{
    hide();
}

void AbEditorPage2::slotShow()
{
    show();
}

void AbEditorPage2::loadFields()
{
    Config cfg( "AddressBook" );
    QString strField;
    int k,
	i = 0;
    cfg.setGroup( "AddressFields" );

    strField = cfg.readEntry( "Field" + QString::number(i++), QString::null );
    while ( !strField.isNull() ) {
	slFields.append(strField);
	strField = cfg.readEntry( "Field" + QString::number(i++), QString::null );
    }

    // hold the indexes of the matches here so we only need to do the
    // difficult matches once...
    int size = slFields.count() - listLabels.count();
    int aMatches[size];
    cfg.setGroup( "ImportantCategory" );
    for ( i = 0; i < size; i++ ) {
	aMatches[i] = slFields.findIndex( cfg.readEntry( "Category" + QString::number(i), QString::null ) );
    }

    QListIterator<QLabel> itLabels( listLabels );
    bool bIsMatch;
    for ( i = 0; *itLabels && i < int(slFields.count()); i++ ) {
	// we must first determine if this is a repeat, if it isn't
	// then it is okay to insert it...
	bIsMatch = FALSE;
	for( k = 0; k < size; k++ ) {
	    if ( aMatches[k] == i ) {
		bIsMatch = TRUE;
		// breakout, there is not reason to keep looking
		break;
	    }
	}
	if ( !bIsMatch ) {
	    (*itLabels)->setText( slFields[i] );
	    ++itLabels;
	}
    }
}

QList<QLineEdit> AbEditorPage2::txts()
{
    return listTxt;
}
