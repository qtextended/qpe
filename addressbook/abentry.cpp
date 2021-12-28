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
#include "abentry.h"
#include <qdict.h>
#include <qfile.h>
#include <qregexp.h>
#include <qtextcodec.h>
#include <qmap.h>
#include <xmlreader.h>
#include <qdatetime.h>

////////////////////////////
// AbEntryList methods

AbEntryList::AbEntryList( )
{
    kkey = 0;
    setAutoDelete( true );
}

AbEntryList::AbEntryList( const QString &filename )
{
    kkey = 0;
    setAutoDelete( true );
    load( filename );
}

QString AbEntryList::key()
{
    ++kkey;
    return QString().setNum( kkey );
}

QString AbEntryList::insert( AbEntry *item )
{
    QString result = key();
    QDict<AbEntry>::insert( result, item );
    return result;
}

void AbEntryList::unremove( const QString &key, AbEntry *item )
{
    QDict<AbEntry>::insert( key, item );
}

void AbEntryList::save( const QString &filename )
{
    QFile f( filename );
    if ( !f.open( IO_WriteOnly ) )
	return;

    QTextStream t( &f );
    t.setCodec( QTextCodec::codecForName( "UTF-8" ) );

    QDictIterator<AbEntry> it(*this);
    while ( it.current() ) {
	it.current()->save( t );
	++it;
    }

    f.close();
}

void AbEntryList::load( const QString &filename )
{
    QFile f( filename );
    if ( !f.open( IO_ReadOnly ) )
	return;

    QTextStream t( &f );
    t.setCodec( QTextCodec::codecForName( "UTF-8" ) );
    clear();

    while ( !t.eof() )
	QDict<AbEntry>::insert( key(), new AbEntry( t ));

    f.close();
}

static bool needsBackslash( const QString &s )
{
    register const QChar *uc;
    uc = s.unicode();
    int n = s.length();
    while ( n-- && *uc != '\\' && *uc != '"' )
	uc++;
    return n >= 0;
}

static QString backslashify( const QString &s )
{
    //inefficient, for now
    QString b;
    int n = s.length();
    int i = 0;
    while ( i < n ) {
	if ( s[i] == '\\' || s[i] == '"' )
	    b += '\\';
	b += s[i++];
    }
    return b;
}

void AbEntryList::saveXML( const QString & filename )
{
    QFile f( filename );
    if ( !f.open( IO_WriteOnly ) )
	return;

    QString out;
    out = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><!DOCTYPE Addressbook ><AddressBook>\n";
    out += " <Groups>\n";
    out += " </Groups>\n";
    out += " <Contacts>\n";

    QDictIterator<AbEntry> it(*this);
    AbEntry *entry;
    while ( (entry = it.current()) ) {
	QDictIterator<QString> entryIt( entry->dict );
	out += "<Contact ";
	const QString *value;
	while ( (value = entryIt.current()) ) {
	    if ( !value->isEmpty() )
		out += entryIt.currentKey() + "=\"" + 
		       (needsBackslash(*value) ? backslashify(*value) : *value)
		       + "\" ";
	    ++entryIt;
	}
	out += "/>\n";
	QCString cstr = out.utf8();
	f.writeBlock( cstr.data(), cstr.length() );
	out = "";
	++it;
    }

    out += " </Contacts>\n</AddressBook>\n";

    QCString cstr = out.utf8();
    f.writeBlock( cstr.data(), cstr.length() );
    
#if 0
    
    QTextStream ts( &f );
    ts.setCodec( QTextCodec::codecForName( "UTF-8" ) );

    ts << "<?xml version=\"1.0\" encoding=\"UTF-8\"?><!DOCTYPE Addressbook ><AddressBook>" << endl;
    ts << " <Groups>" << endl;
    ts << " </Groups>" << endl;
    ts << " <Contacts>" << endl;

    QDictIterator<AbEntry> it(*this);
    while ( it.current() ) {
	it.current()->saveXML( ts );
	++it;
    }

    ts << " </Contacts>" << endl;
    ts << "</AddressBook>" << endl;#
#endif

    f.close();
}
void AbEntryList::loadXML( const QString & filename )
{
    //qDebug ( " AbEntryList::loadXML" );
    QFile f( filename );
    if ( !f.open( IO_ReadOnly ) )
	return;

    QByteArray ba = f.readAll();
    QString data = QString::fromUtf8( ba.data() );
    QChar *uc = (QChar *)data.unicode();
    int len = data.length();
    
//     QTime t;
//     t.start();
    int i = 0;
    while ( (i = data.find( "<Contact ",  i)) != -1 ) {
	// new Contact
	AbEntry *entry = new AbEntry();
	
	i += 9;
	while ( 1 ) {
	    while ( i < len && (uc[i] == ' ' || uc[i] == '\n' || uc[i] == '\r') )
		i++;
	    if ( i >= len-2 || (uc[i] == '/' && uc[i+1] == '>') )
		break;
	    // we have another attribute read it.
	    int j = i;
	    while ( j < len && uc[j] != '=' )
		j++;
	    QString attr = QConstString( uc+i, j-i ).string();
	    //qDebug("attr=%s", attr.latin1() );
	    i = ++j; // skip =
	    while ( i < len && uc[i] != '"' )
		i++;
	    j = ++i;
	    while ( j < len && uc[j] != '"' && uc[j] != '\\' )
		j++;
	    QString value = QConstString( uc+i, j-i ).string();
	    i = j + 1;
	    if ( i < len && uc[i-1] == '\\' ) {
		//This shouldn't happen too often, so not optimized
		bool bs = TRUE;
		while ( i < len ) {
		    QChar ch = uc[i++];
		    if ( bs ) {
			bs = FALSE;
			value += ch;
		    } else if ( ch == '\\' ) {
			bs = TRUE;
		    } else if ( ch == '"' ) {
			break;
		    } else {
			value += ch;
		    }
		}
	    }
	    //qDebug("value=%s", value.latin1() );
	    entry->dict.replace( attr, new QString(value) );
	}
	//qDebug("inserting entry");
	insert( entry );
    }
    //qDebug("after parse: %d", t.elapsed() );

#if 0
    QTextStream ts( &f );
    ts.setCodec( QTextCodec::codecForName( "UTF-8" ) );

    QXmlInputSource inputSource( ts );
    QXmlSimpleReader reader;
    XmlHandler handler;

    reader.setFeature( "http://trolltech.com/xml/features/report-whitespace-only-CharData", FALSE );
    reader.setContentHandler( &handler );
//     QTime t;
//     t.start();
    reader.parse( inputSource );
//    qDebug("after parse: %d", t.elapsed() );

    Node *n = handler.firstNode();
    while ( n ) {

	if ( n->tagName() == "Contact" ) {

	    QDict<AbEntry>::insert( key(), new AbEntry( n ));
            n = n->nextNode();
            continue;
        }
        if ( n->lastChild() ) {
            n = n->firstChild();
        } else {
            if ( !n->nextNode() && n->parentNode() )
                n = n->parentNode();
            n = n->nextNode();
        }
    }
    //   qDebug("end: %d", t.elapsed() );
#endif

    f.close();
}


////////////////////////
// AbEntry methods

AbEntry::AbEntry()
{
    dict.setAutoDelete( true );
}

AbEntry::AbEntry( const AbEntry &r )
{
    QDictIterator<QString> it( r.dict );

    while ( it.current() ) {
	dict.replace( it.currentKey(), new QString( *it.current() ));
	++it;
    }
}

AbEntry& AbEntry::operator=( const AbEntry &r )
{
    if ( this != &r ) {
	dict.clear();
	QDictIterator<QString> it( r.dict );

	while ( it.current() ) {
	    dict.replace( it.currentKey(), new QString( *it.current() ));
	    ++it;
	}
    }
    return *this;
}

AbEntry::AbEntry( QTextStream &t )
{
    dict.setAutoDelete( true );
    load( t );
}

AbEntry::AbEntry( Node *n )
{
    dict.setAutoDelete( true );
    loadXML( n );
}

QStringList AbEntry::custom() const
{
    QStringList result;
    QDictIterator<QString> it( dict );

    while ( it.current() ) {
	if (it.currentKey().find( "X-CUSTOM-", 0 ) == 0)
	    result << it.currentKey();
	++it;
    }
    return result;
}

void AbEntry::save( QTextStream &t )
{
    QDictIterator<QString> it( dict );
    QRegExp reg("\n");

    while ( it.current() ) {
	if ( (it.currentKey().find( ".", 0 ) != 0) &&
	     (!(*it.current()).isEmpty()) ) {
	  t << " " << it.currentKey() << "\n";
	    QString tmp = *it.current();
	    tmp.replace( reg, "\n " );
	    t << " " << tmp << "\n[EOR]\n";
	}
	++it;
    }
    t << "[EOS]\n";
}



// NOT USED
void AbEntry::saveXML( QTextStream &ts )
{
    qDebug( "AbEntry::saveXML" );
    QDictIterator<QString> it( dict );

    ts << "  <Contact ";
    while ( it.current() ) {
	if ( (it.currentKey().find( ".", 0 ) != 0) && (!(*it.current()).isEmpty()) ) {
	    ts << " " << it.currentKey() << "=\"";
	    if ( needsBackslash( *it.current() ) )
		ts << backslashify( *it.current() );
	    else
		ts << *it.current();
	   ts << "\"";
	}
	++it;
    }
    ts << "/>" << endl;
}

void AbEntry::load( QTextStream &t )
{
    while ( !t.eof() ) {
	QString name = t.readLine();
	if ( name == "[EOS]" )
	    break;
	name = name.mid(1);
	QString tmp = t.readLine();
	QString value = "";
	while ( tmp != QString( "[EOR]" )) {
	    if ( !value.isEmpty() )
		value += "\n";
	    value += tmp.mid(1);
	    tmp = t.readLine();
	}
	if ( (name != "") && (value != "") ) {

	    if ( name == "Role" ) name = "JobTitle";
	    if ( name == "AdressStreet" ) name = "BusinessStreet";
	    if ( name == "AdressCity" ) name = "BusinessCity";
	    if ( name == "AdressZip" ) name = "BusinessZip";
	    if ( name == "AdressState" ) name = "BusinessState";
	    if ( name == "AdressCountry" ) name = "BusinessCountry";

	    dict.replace( name, new QString( value ));
	}
    }

    const uint numEdits = 6;
    int fo[numEdits] = { 4, 8, 3, 1, 0, 6 };

    const QString *fieldOrder = find( "X-FieldOrder" );
    if (fieldOrder)
        sscanf( fieldOrder->local8Bit(), "%d,%d,%d,%d,%d,%d",
		&fo[0], &fo[1], &fo[2], &fo[3], &fo[4], &fo[5] );


    for ( unsigned int k = 0; k < numEdits; k++ ) {

	const QString *value = find( QString::number( k ) );
	if ( value && !(*value).isEmpty() ) {
	    QString name;

	    printf( "found: %s\n", value->latin1() );

	    int field = fo[k];

	    switch ( field ) {
		case 0:
		    name = "Birthday";
		    break;
		case 1:
		    name = "DefaultEmail";
		    break;
		case 2:
		    name = "BusinessFax";
		    break;
		case 3:
		    name = "HomePhone";
		    break;
		case 4:
		    name = "BusinessMobile";
		    break;
		case 5:
		    name = "Other";
		    break;
		case 6:
		    name = "Spouse";
		    break;
		case 7:
		    name = "BusinessWebPage";
		    break;
		case 8:
		default:
		    name = "BusinessPhone";
		    break;
	    }
	    dict.replace( name, new QString( *value ));
	    dict.remove( QString::number( k ) );
	}
    }

    dict.remove( "X-FieldOrder" );
}

void AbEntry::loadXML( Node *n )
{
    if ( !n ) return;

    QMap<QString, QString> attributes = n->attributeMap();
    QMap<QString, QString>::ConstIterator it;
    for( it = attributes.begin(); it != attributes.end(); ++it )
	dict.replace( it.key(), new QString( it.data() ) );
}

void AbEntry::insert( const QString key, const QString *item )
{
    if ( item && (*item == "") )
	return;
    dict.insert( key, item );
}

void AbEntry::replace( const QString key, const QString *item )
{
    QString *current = dict.find( key );
    if ( item ) {
	if ( current ) {
	    if ( *item != *current ) {
		if ( *item == "" )
		    dict.remove( key ); // temporary?
		else
		    dict.replace( key, item );
	    }
	}
	else { // (item && !current)
	    dict.replace( key, item );
	}
    }
    else
	debug( "Error:"
	       " AbEntry::replace( const QString, const QString* ) "
	       "passed null item" );
}

bool AbEntry::remove( const QString key )
{
    if ( dict.remove( key ) ) {
	return TRUE;
    } else {
	return FALSE;
    }
}

void AbEntry::touch()
{
    // FIXME
}

const QString *AbEntry::find ( const QString & key ) const
{
    return dict.find( key );
}

const QString *AbEntry::operator[] ( const QString & key ) const
{
    return dict[key];
}

void AbEntry::clear ()
{
    dict.clear();
}
