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
#include "todoentryimpl.h"

#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qtoolbutton.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qmultilineedit.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qapplication.h>

#include <datebookmonth.h>
#include <global.h>
#include <imageedit.h>
#include <organizer.h>

/*
 *  Constructs a NewTaskDialog which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
NewTaskDialog::NewTaskDialog( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : NewTaskDialogBase( parent, name, modal, fl ), todo( 0 ), date( QDate::currentDate() )
{
    QPopupMenu *m1 = new QPopupMenu( this );
    picker = new DateBookMonth( m1, 0, TRUE );
    m1->insertItem( picker );
    buttonDate->setPopup( m1 );
    buttonDate->setPopupDelay( 0 );

    connect( picker, SIGNAL( dateClicked( const QString & ) ),
             this, SLOT( dateChanged( const QString & ) ) );
    connect( picker, SIGNAL( dateClicked( int, int, int ) ),
             this, SLOT( dateChanged( int, int, int ) ) );

    buttonDate->setText( QDate::currentDate().toString() );
}

/*
 *  Destroys the object and frees any allocated resources
 */
NewTaskDialog::~NewTaskDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

void NewTaskDialog::setCategories( const QStringList &c )
{
    comboCategory->insertStringList( c );
}

/*
 * protected slot
 */
void NewTaskDialog::dateChanged( const QString &s )
{
    buttonDate->setText( s );
}

void NewTaskDialog::dateChanged( int y, int m, int d )
{
    date = QDate( y, m, d );
}

/*!
*/

Todo * NewTaskDialog::todoEntry()
{
    if ( !todo )
        todo = new Todo;
    todo->setDate( checkDate->isChecked() );
    todo->setDate( date );
    todo->setCategory( comboCategory->currentText() );
    todo->setPriority( comboPriority->currentItem() + 1 );
    todo->setCompleted( checkCompleted->isChecked() );

    todo->setNotes( txtTodo->text() );

    return todo;
}


/*!

*/

void NewTaskDialog::init( Todo *todo )
{
    int i;

    for ( i = 0; i < comboCategory->count(); ++i ) {
        if ( comboCategory->text( i ) == todo->category() ) {
            comboCategory->setCurrentItem( i );
            break;
        }
    }

    comboPriority->setCurrentItem( todo->priority() - 1 );

    checkCompleted->setChecked( todo->isCompleted() );
    checkDate->setChecked( todo->hasDate() );
    buttonDate->setText( todo->date().toString() );

    picker->setDate( todo->date().year(), todo->date().month(),
                     todo->date().day() );

    txtTodo->setText( todo->notes() );
}

void NewTaskDialog::accept()
{
    QString strText = txtTodo->text();
    if ( !strText || strText == "") {
       // hmm... just decline it then, the user obviously didn't care about it
       QDialog::reject();
       return;
    }
    QDialog::accept();
}
