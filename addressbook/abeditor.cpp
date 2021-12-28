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
#include "abeditor.h"

#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qscrollview.h>
#include <qtoolbutton.h>
#include <qwidgetstack.h>
#include <qmainwindow.h>

#include <config.h>

#include "abeditorbase.h"
#include "abeditorpage2.h"
#include "abentry.h"


static inline bool containsAlphaNum( const QString &str );
static inline bool constainsWhiteSpace( const QString &str );

/*!
  \class AbEditor abeditor.h

  \brief AbEditor is a widget for editing an address book entry.
*/

class AbEditorPage1 : public AbEditorBase
{
public:
    AbEditorPage1( QWidget *parent = 0, const char *name = 0, WFlags fl = 0 );
    ~AbEditorPage1();

    QStringList fields();
    QStringList values();
    QList<QLineEdit> txts();
    void loadFields();

private:
    void init();
    QList<QLabel> listFields;
    QList<QLineEdit> listValue;
};

AbEditorPage1::AbEditorPage1( QWidget* parent, const char* name, WFlags fl )
    : AbEditorBase( parent, name, fl )
{
    init();
}

AbEditorPage1::~AbEditorPage1()
{
}

QStringList AbEditorPage1::fields()
{
    QListIterator<QLabel> it( listFields );
    QStringList sl;
    for ( ; *it; ++it )
	sl.append( (*it)->text() );
    return sl;
}

QStringList AbEditorPage1::values()
{
    QListIterator<QLineEdit> it( listValue );
    QStringList sl;
    for ( ; *it; ++it )
	sl.append( (*it)->text() );
    return sl;
}

QList<QLineEdit> AbEditorPage1::txts()
{
    return listValue;
}

void AbEditorPage1::init()
{
    
    listFields.setAutoDelete( FALSE );
    listValue.setAutoDelete( FALSE );

    // add in our labels
    listFields.append( lblField1 );
    listFields.append( lblField2 );
    listFields.append( lblField3 );
    listFields.append( lblField4 );
    listFields.append( lblField5 );
    listFields.append( lblField6 );
    listFields.append( lblField7 );
    listFields.append( lblField8 );

    listValue.append( txtField1 );
    listValue.append( txtField2 );
    listValue.append( txtField3 );
    listValue.append( txtField4 );
    listValue.append( txtField5 );
    listValue.append( txtField6 );
    listValue.append( txtField7 );
    listValue.append( txtField8 );
    loadFields();
}

void AbEditorPage1::loadFields()
{
    // okay, set the labels up correctly...
    Config cfg( "AddressBook" );
    cfg.setGroup( "ImportantCategory" );

    QListIterator<QLabel> itFields( listFields );
    int i;
    for ( i = 0; *itFields; ++itFields, i++ ) {
	(*itFields)->setText( cfg.readEntry( "Category" + QString::number(i),
					     QString::null ) );
    }
}


AbEditor::AbEditor( AbEntry *entry, QWidget *parent = 0, const char *name = 0,
		    WFlags fl = 0 )
    : QDialog( parent, name, TRUE, fl )
{
    init();
    initMap();
    setEntry( entry );
}

AbEditor::~AbEditor()
{
}

void AbEditor::init()
{
    QVBoxLayout *vb = new QVBoxLayout( this );
    svPage = new QScrollView( this );
    svPage->setHScrollBarMode( QScrollView::AlwaysOff );
    vb->addWidget( svPage );
    svPage->setResizePolicy( QScrollView::AutoOneFit );
    staEditor = new QWidgetStack( svPage->viewport() );

    frmPage1 = new AbEditorPage1( staEditor );
    staEditor->addWidget( frmPage1, 0 );

    frmPage2 = new AbEditorPage2( staEditor );
    staEditor->addWidget( frmPage2, 1 );

    svPage->addChild( staEditor );

    connect( frmPage1->cmdMore, SIGNAL(clicked()), this, SLOT(slotPage2()) );
    connect( frmPage2->cmdBack, SIGNAL(clicked()), this, SLOT(slotPage1()) );


    dlgNote = new QDialog( this, "Note Dialog", TRUE );
    dlgNote->setCaption( tr("Enter Note") );
    QVBoxLayout *vbNote = new QVBoxLayout( dlgNote );
    QLabel *lblNote;
    lblNote = new QLabel( tr("Enter a small note for this contact"), dlgNote );
    lblNote->setMinimumSize( lblNote->sizeHint() );
    vbNote->addWidget( lblNote );
    txtNote = new QMultiLineEdit( dlgNote );
    vbNote->addWidget( txtNote );
    connect( frmPage1->cmdNote, SIGNAL(clicked()), this, SLOT(slotNote()) );
    connect( frmPage2->cmdNote, SIGNAL(clicked()), this, SLOT(slotNote()) );
}

void AbEditor::initMap()
{
    // since the fields and the XML fields exist, create a map 
    // between them...
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
	mapField.insert( strCfg1, strCfg2 );
	strCfg1 = cfg1.readEntry( "Field" + QString::number(i), 
				  QString::null );
	strCfg2 = cfg2.readEntry( "XMLField" + QString::number(i++), 
				  QString::null );
    }
    // The fields are pretty static, so we may as well get them
    slFields = frmPage1->fields() + frmPage2->fields();
}

void AbEditor::resizeEvent( QResizeEvent *e )
{
    //Quick hack
    if ( e->size().height() > e->oldSize().height() 
	 && staEditor-> visibleWidget() == frmPage1 )
	slotPage1();
}


void AbEditor::slotPage1()
{
    staEditor->setMaximumSize( width(), svPage->visibleHeight() );
    staEditor->raiseWidget( frmPage1 );
}

void AbEditor::slotPage2()
{
    staEditor->setMaximumSize( svPage->visibleWidth(),
			       frmPage2->sizeHint().height() );
    staEditor->raiseWidget( frmPage2 );
}

void AbEditor::setEntry( AbEntry* entry )
{
    ent = entry;
    QList<QLineEdit> myTxts = frmPage1->txts();
    QListIterator<QLineEdit> it( myTxts );
    const QString *str;
    str = ent->find( "FirstName" );
    if ( str )
	frmPage1->txtFirstName->setText( *str );
    else
	frmPage1->txtFirstName->setText( "" );
    str = ent->find( "LastName" );
    if ( str )
	frmPage1->txtLastName->setText( *str );
    else
	frmPage1->txtLastName->setText( "" );

    // okay, do the first 8 fields and then the rest...
    int i;

    for( i = 0; i < 8; i++ ) {
	str = ent->find( mapField[slFields[i]] );
	if ( str )
	    (*it)->setText( *str );
	else
	    (*it)->setText( "" );  // reset the text
	++it;
    }
    myTxts = frmPage2->txts();
    QListIterator<QLineEdit> it2( myTxts );
    for ( ; *it2; ++it2 ) {
	str = ent->find( mapField[slFields[i++]] );
	if ( str )
	    (*it2)->setText( *str );
	else
	    (*it2)->setText( "" );
    }

    str = ent->find( "Notes" );
    if ( str )
	txtNote->setText( *str );
    else
	txtNote->setText( "" );
}

void AbEditor::parseName()
{
     ent->replace( "FirstName", new QString( frmPage1->txtFirstName->text() ) );
     ent->replace( "LastName", new QString( frmPage1->txtLastName->text() ) );
     ent->replace( "FullName", new QString( *ent->find("FirstName") + " "
					    + *ent->find("LastName") ) );
     ent->replace( "FileAs", new QString( *ent->find( "FullName" ) ) );
}

void AbEditor::accept()
{
    if ( isEmpty() )
	reject();
    else {
	saveEntry();
	QDialog::accept();
    }
}
	
bool AbEditor::isEmpty()
{
    // analyze all the fields and make sure there is _something_ there
    // that warrants saving...
    slValues = frmPage1->values() + frmPage2->values();
    slValues.append( frmPage1->txtFirstName->text() );
    slValues.append( frmPage1->txtLastName->text() );
    slValues.append( txtNote->text() );

    QStringList::Iterator it;
    for ( it = slValues.begin(); it != slValues.end(); ++it ) {
	if ( !(*it).isEmpty() && !(*it).isNull() && containsAlphaNum(*it) )
	    return FALSE;
    }
    return TRUE;
}

void AbEditor::saveEntry()
{
    int count,
	i;
    slValues.clear();
    slValues = frmPage1->values() + frmPage2->values();

    count = slFields.count();

    for ( i = 0; i < count; i++ ) {
	if ( slValues[i].isEmpty() ) {
	    if ( ent->find( mapField[slFields[i]] ) )
		ent->remove( mapField[slFields[i]] );
	} else
	    ent->replace( mapField[ slFields[i] ], new QString(slValues[i]) );
    }

    // parsename is always called afterwards, so we don't have to worry
    // about that
    QString str = txtNote->text();
    if ( !str.isNull() )
	ent->replace( "Notes", new QString(str) );
}

void AbEditor::slotNote()
{
    dlgNote->showMaximized();
    dlgNote->exec();
}

void AbEditor::loadFields()
{
    frmPage1->loadFields();
    frmPage2->loadFields();
    slFields.clear();
    slFields = frmPage1->fields() + frmPage2->fields();
}


static inline bool containsAlphaNum( const QString &str )
{
    int i,
	count = str.length();
    for ( i = 0; i < count; i++ )
	if ( !str[i].isSpace() )
	    return TRUE;
    return FALSE;
}

static inline bool constainsWhiteSpace( const QString &str )
{
    int i,
	count = str.length();
    for (i = 0; i < count; i++ )
	if ( str[i].isSpace() )
	    return TRUE;
    return FALSE;
}

