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
#include "todotable.h"

#include <qpainter.h>
#include <qcombobox.h>
#include <qfile.h>
#include <qcursor.h>
#include <qtextstream.h>
#include <qtextcodec.h>
#include <qtimer.h>
#include <qxml.h>

#include <xmlreader.h>

CheckItem::CheckItem( QTable *t )
    : QTableItem( t, Never, "" ), checked( FALSE )
{
}

QString CheckItem::key() const
{
    return checked ? "2" : "1";
}

void CheckItem::setChecked( bool b )
{
    checked = b;
    table()->updateCell( row(), col() );
}

void CheckItem::toggle()
{
    checked = !checked;
    table()->updateCell( row(), col() );
}

bool CheckItem::isChecked() const
{
    return checked;
}

static const int BoxSize = 10;

void CheckItem::paint( QPainter *p, const QColorGroup &cg, const QRect &cr, bool )
{
    p->fillRect( 0, 0, cr.width(), cr.height(), cg.brush( QColorGroup::Base ) );

    int marg = ( cr.width() - BoxSize ) / 2;
    int x = 0;
    int y = ( cr.height() - BoxSize ) / 2;
    p->setPen( QPen( cg.text() ) );
    p->drawRect( x + marg, y, BoxSize, BoxSize );
    p->drawRect( x + marg+1, y+1, BoxSize-2, BoxSize-2 );
    p->setPen( darkGreen );
    x += 1;
    y += 1;
    if ( checked ) {
	QPointArray a( 7*2 );
	int i, xx, yy;
	xx = x+1+marg;
	yy = y+2;
	for ( i=0; i<3; i++ ) {
	    a.setPoint( 2*i,   xx, yy );
	    a.setPoint( 2*i+1, xx, yy+2 );
	    xx++; yy++;
	}
	yy -= 2;
	for ( i=3; i<7; i++ ) {
	    a.setPoint( 2*i,   xx, yy );
	    a.setPoint( 2*i+1, xx, yy+2 );
	    xx++; yy--;
	}
	p->drawLineSegments( a );
    }
}


ComboItem::ComboItem( QTable *t, EditType et )
    : QTableItem( t, et, "3" ), cb( 0 )
{
    setReplaceable( FALSE );
}

QWidget *ComboItem::createEditor() const
{
    QString txt = text();
    ( (ComboItem*)this )->cb = new QComboBox( table()->viewport() );
    cb->insertItem( "1" );
    cb->insertItem( "2" );
    cb->insertItem( "3" );
    cb->insertItem( "4" );
    cb->insertItem( "5" );
    cb->setCurrentItem( txt.toInt() - 1 );
    return cb;
}

void ComboItem::setContentFromEditor( QWidget *w )
{
    if ( w->inherits( "QComboBox" ) )
	setText( ( (QComboBox*)w )->currentText() );
    else
	QTableItem::setContentFromEditor( w );
}

void ComboItem::setText( const QString &s )
{
    if ( cb )
	cb->setCurrentItem( s.toInt() - 1 );
    QTableItem::setText( s );
}

QString ComboItem::text() const
{
    if ( cb )
	return cb->currentText();
    return QTableItem::text();
}



TodoTable::TodoTable( QWidget *parent, const char *name )
    : QTable( 0, 3, parent, name ), showComp( true ), enablePainting( true )
{
    setSorting( TRUE );
    setSelectionMode( NoSelection );
    setColumnStretchable( 2, TRUE );
    setColumnWidth( 0, 20 );
    setColumnWidth( 1, 35 );
    setLeftMargin( 0 );
    verticalHeader()->hide();
    horizontalHeader()->setLabel( 0, tr( "C." ) );
    horizontalHeader()->setLabel( 1, tr( "Prior." ) );
    horizontalHeader()->setLabel( 2, tr( "Description" ) );
    connect( this, SIGNAL( clicked( int, int, int, const QPoint & ) ),
	     this, SLOT( slotClicked( int, int, int, const QPoint & ) ) );
    connect( this, SIGNAL( pressed( int, int, int, const QPoint & ) ),
	     this, SLOT( slotPressed( int, int, int, const QPoint & ) ) );
    connect( this, SIGNAL( valueChanged( int, int ) ),
             this, SLOT( slotCheckPriority( int, int ) ) );
    connect( this, SIGNAL( currentChanged( int, int ) ),
             this, SLOT( slotCurrentChanged( int, int ) ) );

    menuTimer = new QTimer( this );
    connect( menuTimer, SIGNAL(timeout()), this, SLOT(slotShowMenu()) );
}

void TodoTable::addEntry( Todo *todo )
{
    int row = numRows();
    setNumRows( row + 1 );
    CheckItem *ci = new CheckItem( this );
    setItem( row, 0, ci );
    ci->setChecked( todo->isCompleted() );
    ComboItem *co = new ComboItem( this, QTableItem::WhenCurrent );
    setItem( row, 1, co );
    co->setText( QString::number( todo->priority() ) );

    QTableItem *ti = new QTableItem( this, QTableItem::Never, todo->description() );
    ti->setReplaceable( false );
    setItem( row, 2, ti );

    todoList.insert( ci, todo );
    updateVisible();
}

void TodoTable::slotClicked( int row, int col, int, const QPoint &pos )
{
    // let's switch on the column number...
    CheckItem *i;
    int x,
        y,
        w,
        h;
    switch ( col )
    {
        case 0:
            i = static_cast<CheckItem*>(item( row, col ));
            x = pos.x() - columnPos( col );
            y = pos.y() - rowPos( row );
            w = columnWidth( col );
            h = rowHeight( row );
            if ( i && x >= ( w - BoxSize ) / 2 && x <= ( w - BoxSize ) / 2 + BoxSize &&
                y >= ( h - BoxSize ) / 2 && y <= ( h - BoxSize ) / 2 + BoxSize ) {
                i->toggle();
            }
            emit signalDoneChanged( i->isChecked() );
            break;
        case 1:
            break;
        case 2:
            // may as well edit it...
	    menuTimer->stop();
            emit signalEdit();
            break;
    }

}

void TodoTable::slotPressed( int, int col, int, const QPoint &pos )
{
    if ( col == 2 )
	menuTimer->start( 750, TRUE );
}

void TodoTable::slotShowMenu()
{
    emit signalShowMenu( QCursor::pos() );
}

void TodoTable::slotCurrentChanged( int, int )
{
    menuTimer->stop();
}

Todo *TodoTable::currentEntry() const
{
    QTableItem *i = item( currentRow(), 0 );
    if ( !i )
        return 0;
    Todo *todo = *todoList.find( (CheckItem*)i );
    todo->setCompleted( ( (CheckItem*)item( currentRow(), 0 ) )->isChecked() );
    todo->setPriority( ( (ComboItem*)item( currentRow(), 1 ) )->text().toInt() );
    return todo;
}

void TodoTable::replaceEntry( Todo *todo )
{
    int row = currentRow();
    todoList.remove( (CheckItem*)item( row, 0 ) );
    ( (CheckItem*)item( row, 0 ) )->setChecked( todo->isCompleted() );
    ( (ComboItem*)item( row, 1 ) )->setText( QString::number( todo->priority() ) );
    QString strTodo = todo->description();
    item( row, 2 )->setText( strTodo );
    todoList.insert( (CheckItem*)item( row, 0 ), todo );
    updateVisible();
}

void TodoTable::save( const QString &fn )
{
    QFile f( fn );
    if ( !f.open( IO_WriteOnly ) )
        return;
    QTextStream ts( &f );
    ts.setCodec( QTextCodec::codecForName( "UTF-8" ) );
    ts << "<!DOCTYPE Todolist>" << endl;

    ts << " <Tasks>" << endl;
    for ( QMap<CheckItem*, Todo *>::Iterator it = todoList.begin(); it != todoList.end(); ++it ) {
        if ( !item( it.key()->row(), 0 ) )
            continue;
        Todo *todo = *it;
        ts << "  <Task>" << endl;
        todo->save( ts, "    " );
        ts << "  </Task>" << endl;
    }

    ts << "</Tasks>" << endl;
}

void TodoTable::saveCategories( const QString &fn )
{
    QFile f( fn );
    if ( !f.open( IO_WriteOnly ) )
        return;
    QTextStream ts( &f );
    ts.setCodec( QTextCodec::codecForName( "UTF-8" ) );
    ts << "<!DOCTYPE CategoryList>" << endl;

    ts << "<Categories>" << endl;
    for ( QStringList::Iterator cit = categoryList.begin(); cit != categoryList.end(); ++cit )
	ts << "<" << *cit << "/>" << endl;
    ts << "</Categories>" << endl;
}

void TodoTable::load( const QString &fn )
{
    QFile f( fn );
    if ( !f.open( IO_ReadOnly ) )
	return;

    QTextStream ts( &f );
    QXmlInputSource inputsource( ts );
    QXmlSimpleReader reader;
    XmlHandler handler;

    reader.setFeature( "http://trolltech.com/xml/features/report-whitespace-only-CharData", FALSE );
    reader.setContentHandler( &handler );
    reader.parse( inputsource );

    Node *n = handler.firstNode();

    while ( n ) {
	if ( n->tagName() == "Task" ) {
	    Todo *todo = new Todo;
	    todo->load( n->firstChild() );
	    if ( categoryList.find( todo->category() ) == categoryList.end() )
		categoryList.append( todo->category() );
	    addEntry( todo );
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

    sortColumn(2,TRUE,TRUE);
    sortColumn(1,TRUE,TRUE);
    sortColumn(0,TRUE,TRUE);
}

void TodoTable::loadOldFormat( const QString &fn )
{
    QFile f( fn );
    if ( !f.open( IO_ReadOnly ) )
	return;

    QTextStream ts( &f );
    QXmlInputSource inputsource( ts );
    QXmlSimpleReader reader;
    XmlHandler handler;

    reader.setFeature( "http://trolltech.com/xml/features/report-whitespace-only-CharData", FALSE );
    reader.setContentHandler( &handler );
    reader.parse( inputsource );

    Node *n = handler.firstNode();

    while ( n ) {
	if ( n->tagName() == "task" ) {
	    Todo *todo = new Todo;
	    todo->load( n->firstChild() );
	    if ( categoryList.find( todo->category() ) == categoryList.end() )
		categoryList.append( todo->category() );
	    addEntry( todo );
	    n = n->nextNode();
	    continue;
	} else if ( n->tagName() == "categories" ) {
	    Node *cn = n->firstChild();
	    while ( cn ) {
		if ( cn->tagName() == "category" ) {
		    if ( categoryList.find( cn->data() ) == categoryList.end() )
			categoryList.append( cn->data() );
		}
		cn = cn->nextNode();
	    }
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

    if ( categoryList.isEmpty() ) {
	categoryList.append( tr("Private") );
	categoryList.append( tr("Business") );
    }

    sortColumn(2,TRUE,TRUE);
    sortColumn(1,TRUE,TRUE);
    sortColumn(0,TRUE,TRUE);
}

void TodoTable::loadCategories( const QString &fn )
{
    QFile f( fn );
    if ( !f.open( IO_ReadOnly ) )
	return;

    QTextStream ts( &f );
    QXmlInputSource inputsource( ts );
    QXmlSimpleReader reader;
    XmlHandler handler;

    reader.setFeature( "http://trolltech.com/xml/features/report-whitespace-only-CharData", FALSE );
    reader.setContentHandler( &handler );
    reader.parse( inputsource );

    Node *n = handler.firstNode();

    while ( n ) {
	if ( n->tagName() == "Categories" ) {
	    Node *cn = n->firstChild();
	    while ( cn ) {
		if ( categoryList.find( cn->tagName() ) == categoryList.end() )
		    categoryList.append( cn->tagName() );
		cn = cn->nextNode();
	    }
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

    if ( categoryList.isEmpty() ) {
	categoryList.append( tr("Private") );
	categoryList.append( tr("Business") );
    }
}

void TodoTable::updateVisible()
{
    for ( int row = 0; row < numRows(); row++ ) {
	CheckItem *ci = (CheckItem *)item( row, 0 );
	Todo *t = todoList[ci];
	if ( t ) {
	    bool hide = false;
	    if ( !showComp && ci->isChecked() )
		hide = true;
	    if ( !showCat.isEmpty() && t->category() != showCat )
		hide = true;

	    if ( hide ) {
		hideRow( row );
	    } else {
		showRow( row );
		adjustRow( row );
	    }
	}
    }
}

void TodoTable::viewportPaintEvent( QPaintEvent *pe )
{
    if ( enablePainting )
	QTable::viewportPaintEvent( pe );
}

void TodoTable::setPaintingEnabled( bool e )
{
    if ( e != enablePainting ) {
	if ( !enablePainting )
	    viewport()->update();
	enablePainting = e;
    }
}

void TodoTable::clear()
{
    todoList.clear();
    for ( int r = 0; r < numRows(); ++r ) {
	for ( int c = 0; c < numCols(); ++c ) {
            if ( cellWidget( r, c ) )
                clearCellWidget( r, c );
	    clearCell( r, c );
	}
    }
    setNumRows( 0 );
}

void TodoTable::sortColumn( int col, bool ascending, bool /*wholeRows*/ )
{
    // The default for wholeRows is false, however
    // for this todo table we want to exchange complete
    // rows when sorting. Also, we always want ascending, since
    // the values have a logical order.
    QTable::sortColumn( col, TRUE, TRUE );
    updateVisible();
}

void TodoTable::slotCheckPriority(int row, int col )
{
    // kludgey work around to make forward along the updated priority...
    if ( col == 1 ) {
        // let everyone know!!
        ComboItem* i = static_cast<ComboItem*>( item( row, col ) );
        emit signalPriorityChanged( i->text().toInt() );
    }
}
