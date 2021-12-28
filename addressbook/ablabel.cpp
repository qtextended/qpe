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
#include <qregexp.h>

#include "ablabel.h"
#include "abentry.h"

AbLabel::AbLabel( QWidget *parent, const char *name )
  : QTextView( parent, name ), ent( 0 )
{
}

AbLabel::~AbLabel()
{
}

void AbLabel::init( AbEntry *entry )
{
    ent = entry;
}

void AbLabel::sync()
{
    if ( !ent ) return;

    QString text;
    const QString *value, *company, *state;

    // name, jobtitle and company
    if ( (value = ent->find( "FileAs" )) )
	text += "<b>" + *value + "</b><br>";
    if ( (value = ent->find( "JobTitle" )) )
	text += *value + "<br>";

    company =  ent->find( "Company" );
    if ( (value = ent->find( "Department" )) ) {
	text += *value;
	if ( company )
	    text += ", ";
    }
    if (company)
	text += *company + "<br>";

    // business address
    if ( ent->find( "BusinessStreet" ) || ent->find( "BusinessCity" ) ||
	 ent->find( "BusinessZip" ) || ent->find( "BusinessCountry" ) ) {
	text += "<br>";
	text += tr( "<b>Work Address:</b>" );
	text +=  "<br>";
    }
    text += "<br>";
    if ( (value = ent->find( "BusinessStreet" )) )
	text += *value + "<br>";
    state =  ent->find( "BusinessState" );
    if ( (value = ent->find( "BusinessCity" )) ) {
	text += *value;
	if ( state )
	    text += ", " + *state;
	text += "<br>";
    }
    else if (state)
	text += *state + "<br>";
    if ( (value = ent->find( "BusinessZip" )) )
	text += *value + "<br>";
    if ( (value = ent->find( "BusinessCountry" )) )
	text += *value + "<br>";

    // home address
    if ( ent->find( "HomeStreet" ) || ent->find( "HomeCity" ) ||
	 ent->find( "HomeZip" ) || ent->find( "HomeCountry" ) ) {
	text += "<br>";
	text += tr( "<b>Home Address:</b>" );
	text +=  "<br>";
    }

    if ( (value = ent->find( "HomeStreet" )) )
	text += *value + "<br>";
    state =  ent->find( "HomeState" );
    if ( (value = ent->find( "HomeCity" )) ) {
	text += *value;
	if ( state )
	    text += ", " + *state;
	text += "<br>";
    }
    else if (state)
	text += *state + "<br>";
    if ( (value = ent->find( "HomeZip" )) )
	text += *value + "<br>";
    if ( (value = ent->find( "HomeCountry" )) )
	text += *value + "<br>";

    // other fields
    QStringList fields, trfields;
    fields << "DefaultEmail" << "HomePhone" << "HomeFax" << "HomeMobile" << "HomeWebPage"
	   << "BusinessWebPage" << "Office" << "BusinessPhone" << "BusinessFax" << "BusinessMobile"
	   << "BusinessPager" << "Profession" << "Assistant" << "Manager" << "Spouse" << "Gender"
	   << "Birthday" << "Aniversary" << "Nickname";

    trfields << tr( "Email" ) << tr( "Home Ph" ) << tr( "Home Fax" ) << tr( "Home Mb" )
	     << tr( "Home WebPage" ) << tr( "Work WebPage" ) << tr( "Office" )
	     << tr( "Work Ph" ) << tr( "Work Fax" ) << tr( "Work Mb" )
	     << tr( "Work Pager" ) << tr( "Profession" ) << tr( "Assistant" )
	     << tr( "Manager" ) << tr( "Spouse" ) << tr( "Gender" )
	     << tr( "Birthday" ) << tr( "Aniversary" ) << tr( "Nickname" );

    bool first = TRUE;
    QStringList::ConstIterator trit = trfields.begin();
    for ( QStringList::ConstIterator it = fields.begin(); it != fields.end(); ++it, ++trit ) {

	value = ent->find( *it );
	if ( !value || (*value).isEmpty() )
	    continue;

	if ( first ) {
	    text += "<br>";
	    first = FALSE;
	}

	QString s = *value;
	if ( *it == "Gender" )
	    if ( s.toInt() == 1 )
		s = tr( "Male" );
	    else if ( s.toInt() == 0 )
		s = tr( "Female" );
	    else
		s = tr( "Unspecified" );
	text += *trit + ": " + s + "<br>";
    }

    // notes last
    if ( (value = ent->find( "Notes" ) ) )	 {
	QRegExp reg("\n");
	QString tmp = *value;
	tmp.replace( reg, "<br>" );
	text += "<br>" + tmp + "<br>";
    }
    setText( text );
}
