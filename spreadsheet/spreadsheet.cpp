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
#include "spreadsheet.h"
#include "fileselector.h"
#include "global.h"
#include <qtextstream.h>
#include <qregexp.h>
#include <qfileinfo.h>
#include <qtextcodec.h>

// return evaluated token
// token can be a cell location, numerical constant or mathematical operator
QString EquationCell::evalToken( QString& tkn ) const
{
    if ( tkn.length() && tkn[0].isLetter() ) {
	int c = QChar(tkn[0]) - 'A';
	bool ok;
	int r = tkn.mid( 1, tkn.length() ).toInt(&ok);
	if ( ok )
	    r--;
	if ( c >= 0 && ok && r >= 0 ) {
	    if ( r == row() && c == col() ) // avoid infinite loop
		return "ERROR";
	    QTableItem* sc = table()->item( r, c );
	    if ( sc )
		return sc->text();
	}
    }
    return tkn;
}

// return relative precedence of operator c
int EquationCell::precedence( QChar c ) const
{
    if ( c == '+' )
	return 1;
    if ( c == '-' )
	return 1;
    if ( c == '*' )
	return 2;
    if ( c == '/' )
	return 2;
    return 0;
}

// return infix expression s as a postfix expression
// operators and terms are separated by spaces
QString EquationCell::toPostfix( QString s ) const
{
    s = s.replace( QRegExp("\\s"), "" );
    CharStack stack;
    QString currToken;
    QString postfixString;
    for ( uint i = 0; i < s.length(); ++i ) {
	QChar c = s[(int)i];
	if ( c.isLetter() || c.isNumber() )
	    postfixString += c ;
	else {
	    if ( c == '(' )
		stack.push( c );
	    if ( c == ')' && stack.count() ) {
		QChar m = stack.pop();
		while ( stack.count() && m  != '(' ){
		    postfixString += " ";
		    postfixString += m;
		    m = stack.pop();
		}
	    }
	    if ( c == '*' || c == '/' || c == '+' || c == '-' ){
		while ( stack.count() && ( precedence(stack.top()) >= precedence(c) ) ) {
		    QChar m = stack.pop();
		    postfixString += " ";
		    postfixString += m;
		}
		stack.push( c );
		postfixString += " ";
	    }
	}
    }
    while ( stack.count() ) {
	postfixString += " ";
	postfixString += stack.pop();
    }
    return postfixString;
}

// s is a legal postfix expression with tokens separated by spaces
// return result of evaluation as a string
QString EquationCell::eval( QString s ) const
{
    int start = 0;
    int end = 0;
    DoubleStack stack;
    while ( TRUE ) {
	end = s.find( QRegExp("[\\s+\\-*/]"), start );
	if ( end != -1 ) {
	    QString term;
	    if ( end == start ) {
		term = QString(s[start]);
		start = end + 2;
	    } else {
		term = s.mid( start, end-start );
		start = end + 1;
	    }
	    bool ok;
	    double val = evalToken( term ).toDouble(&ok);
	    if ( !ok ) {
		if ( term.find( QRegExp("[+\\-*/]")) != -1 ) {
		    double d2 = stack.pop();
		    double d1 = stack.pop();
		    double d3 = 0.0;
		    if ( term == "+" )
			d3 = d1 + d2;
		    if ( term == "-" )
			d3 = d1 - d2;
		    if ( term == "*" )
			d3 = d1 * d2;
		    if ( term == "/" )
			d3 = d1 / d2;
		    stack.push( d3 );
		} else {
		    return term + QString(":ERROR");
		}
	    } else {
		stack.push( val );
	    }
	} else {
	    break;
	}
    }
    if ( stack.count() )
	return QString::number(stack.pop());
    else
	return s;
}

void EquationCell::paint ( QPainter * p, const QColorGroup & cg, const QRect & cr, bool selected )
{
    setText(eval(toPostfix(eq)));
    QColorGroup mycg(cg);
    mycg.setColor(QColorGroup::Text, Qt::red);
    mycg.setColor(QColorGroup::HighlightedText, cyan);
    QTableItem::paint(p,mycg,cr, selected);
}

///////////

Spreadsheet::Spreadsheet ( QWidget * parent, const char * name )
    : QTable(parent,name),
      editor(0)
{
    editingFile = false;
    setNumRows( 99 );
    setNumCols( 26 );
    setLeftMargin( fontMetrics().width("99 ")+4 );
    for ( int i = 0; i < 26; ++i ) {
	horizontalHeader()->setLabel( i, QChar('A'+i) );
	setColumnWidth( i, fontMetrics().width("x")*10 );
    }
    connect( this, SIGNAL(currentChanged(int,int)),
	     SLOT(newSelection(int,int)));
    connect( this, SIGNAL( newTextSelected(const QString&) ),
	     SLOT( textSelected(const QString&) ) );
}

void Spreadsheet::setEditor( QLineEdit* le )
{
    editor = le;
    editor->installEventFilter( this );
}

bool Spreadsheet::eventFilter( QObject* o, QEvent* e )
{
    if ( o == editor ) {
	if ( e->type() == QEvent::KeyPress ) {
	    QKeyEvent* ke = (QKeyEvent*)e;
	    if ( ke->key() == Key_Return || ke->key() == Key_Enter || ke->key() == Key_Tab || ke->key() == Key_BackTab ) {
		QString eq = editor->text();
		if ( !eq.isEmpty() ) {
		    EquationCell* ec = new EquationCell( eq.upper(), this );
		    setItem( currentRow(), currentColumn(), ec );
		    QString eval = ec->text();
		    emit newTextSelected( eq );
		}
		viewport()->setFocus();
		return TRUE;
	    }
	}
    }
    if ( o == viewport() ) {
	if ( e->type() == QEvent::KeyPress ) {
	    QKeyEvent* ke = (QKeyEvent*)e;
	    if ( ke->key() == Key_Delete ) {
		clearCurrent();
		return TRUE;
	    }
	}
    }
    return QTable::eventFilter( o, e );
}

void Spreadsheet::clearCurrent()
{
    clearCell(currentRow(),currentColumn());
    emit newTextSelected( " " );
}

void Spreadsheet::editEquation()
{
    if ( !editor )
	return;
    editor->setFocus();
    editor->selectAll();
}

void Spreadsheet::newSelection( int row, int col )
{
    QTableItem* i = item( row, col );
    if ( i && i->editType() == QTableItem::Never )
	emit newTextSelected( ((EquationCell*)i)->equation() );
    else if ( i )
	emit newTextSelected( i->text() );
    else
	emit newTextSelected( " " );
}

void Spreadsheet::textSelected( const QString& text )
{
    if ( editor )
	editor->setText( text );
}

void Spreadsheet::setCellContentFromEditor ( int row, int col )
{
    QTable::setCellContentFromEditor( row, col );
    emit newTextSelected( item( row, col )->text() );
}

void Spreadsheet::save()
{
    if ( !editingFile || !itemCount())
	return;
    QByteArray ba;
    QString text;
    bool equation;
    QTextStream ts( ba, IO_WriteOnly );
#if QT_VERSION <= 230 && defined(QT_NO_CODECS)
    // The below should work, but doesn't in Qt 2.3.0
    ts.setCodec( QTextCodec::codecForMib( 106 ) );
#else
    ts.setEncoding( QTextStream::UnicodeUTF8 );
#endif
    for ( int c = 0; c < numCols(); ++c ) {
	for ( int r = 0; r < numRows(); ++r ) {
	    QTableItem* it = item( r, c );
	    if ( it ) {
		if ( it->editType() == QTableItem::Never ) {
		    text = ((EquationCell*)it)->equation();
		    equation = TRUE;
		} else {
		    text= it->text();
		    equation = FALSE;
		}
		ts << r << "\n" << c << "\n" << text << "\n" << equation << "\n";
	    }
	}
    }
    DocLnk f = currentFile;
    // find first suitable name
    for ( int c = 0; c < numCols() && f.name().isEmpty(); ++c ) {
	for ( int r = 0; r < numRows(); ++r ) {
	    QTableItem* itm = item( r, c );
	    if ( itm ) {
		QString text = itm->text();
		bool num = FALSE;
		bool ok1 = FALSE, ok2 = FALSE;
		(void)text.toInt( &ok1 );
		if ( !ok1 )
		    (void)text.toDouble( &ok2 );
		num = ok1 || ok2;
		if ( !num ) {
		    f.setName(text);
		    break;
		}
	    }
	}
    }
    if ( f.name().isEmpty() ) {
	// give up
	f.setName("Sheet");
    }
    FileManager fm;
    fm.saveFile( f, ba );
}

int Spreadsheet::itemCount()
{
    int count = 0;
    for ( int c = 0; c < numCols(); ++c )
	for ( int r = 0; r < numRows(); ++r )
	    if ( item(r,c) )
		count++;
    return count;
}

void Spreadsheet::clear()
{
    for ( int c = 0; c < numCols(); ++c )
	for ( int r = 0; r < numRows(); ++r )
	    clearCell( r, c );
}

void Spreadsheet::closeFile()
{
    save();
    //    clear();
    editingFile = FALSE;
}

void Spreadsheet::newFile( const DocLnk & f )
{
    save();
    clear();
    currentFile = f;
    currentFile.setType("application/x-qpe-ss");
    editingFile = TRUE;
    viewport()->setFocus();
}

void Spreadsheet::openFile( const DocLnk & f )
{
    clear();
    FileManager fm;
    QByteArray ba;
    if ( !fm.loadFile( f, ba ) )
	return;
    QTextStream ts( ba, IO_ReadOnly );
#if QT_VERSION <= 230 && defined(QT_NO_CODECS)
    // The below should work, but doesn't in Qt 2.3.0
    ts.setCodec( QTextCodec::codecForMib( 106 ) );
#else
    ts.setEncoding( QTextStream::UnicodeUTF8 );
#endif
    QString txt = ts.readLine();
    while ( txt != QString::null ) {
	int r = txt.toInt(); txt = ts.readLine();
	int c = txt.toInt(); txt = ts.readLine();
	QString data = txt;  txt = ts.readLine();
	bool equation = txt.toInt();txt = ts.readLine();
	if ( equation )
	    setItem( r, c, new EquationCell( data, this) );
	else
	    setItem( r, c, new SpreadsheetCell( data, this) );
    }
    currentFile = f;
    editingFile = TRUE;
    viewport()->setFocus();
}
