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
#include <qpeapplication.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qdir.h>
#include <stdlib.h>
#include "inputmethods.h"
#include "config.h"

#ifdef Q_WS_QWS
#include <qwindowsystem_qws.h>
#include <qwsevent_qws.h>
#endif

#include <inputmethodinterface.h>
#include <qlibrary.h>

#ifdef SINGLE_APP
#include "handwritingimpl.h"
#include "keyboardimpl.h"
#include "pickboardimpl.h"
#endif


/* XPM */
static const char * tri_xpm[]={
"9 9 2 1",
"a c #000000",
". c None",
".........",
".........",
".........",
"....a....",
"...aaa...",
"..aaaaa..",
".aaaaaaa.",
".........",
"........."};


static const int inputWidgetStyle = QWidget::WStyle_Customize | 
				    QWidget::WStyle_Tool |
				    QWidget::WStyle_StaysOnTop |
				    QWidget::WGroupLeader;

InputMethods::InputMethods( QWidget *parent ) :
    QWidget( parent, 0, WStyle_Tool | WStyle_Customize )
{
    method = NULL;

    QHBoxLayout *hbox = new QHBoxLayout( this );

    kbdButton = new QToolButton( this );
    kbdButton->setFocusPolicy(NoFocus);
    kbdButton->setToggleButton( TRUE );
    kbdButton->setFixedHeight( 17 );
    kbdButton->setFixedWidth( 32 );
    kbdButton->setAutoRaise( TRUE );
    kbdButton->setUsesBigPixmap( TRUE );
    hbox->addWidget( kbdButton );
    connect( kbdButton, SIGNAL(toggled(bool)), this, SLOT(showKbd(bool)) );

    kbdChoice = new QToolButton( this );
    kbdChoice->setFocusPolicy(NoFocus);
    kbdChoice->setPixmap( QPixmap( (const char **)tri_xpm ) );
    kbdChoice->setFixedHeight( 17 );
    kbdChoice->setFixedWidth( 12 );
    kbdChoice->setAutoRaise( TRUE );
    hbox->addWidget( kbdChoice );
    connect( kbdChoice, SIGNAL(clicked()), this, SLOT(chooseKbd()) );

    connect( (QPEApplication*)qApp, SIGNAL(clientMoused()),
	    this, SLOT(resetStates()) );

    loadInputMethods();
}

void InputMethods::hideInputMethod()
{
    kbdButton->setOn( FALSE );
}

void InputMethods::showInputMethod()
{
    kbdButton->setOn( TRUE );
}

void InputMethods::resetStates()
{
    if ( method )
	method->interface->resetState();
}

QRect InputMethods::inputRect() const
{
    if ( !method || !method->widget->isVisible() )
	return QRect();
    else
	return method->widget->geometry();
}

void InputMethods::loadInputMethods()
{
#ifndef SINGLE_APP
    hideInputMethod();
    method = 0;

    QValueList<InputMethod>::Iterator mit;
    for ( mit = inputMethodList.begin(); mit != inputMethodList.end(); ++mit ) {
	(*mit).interface->release();
	(*mit).library->unload();
	delete (*mit).library;
    }
    inputMethodList.clear();

    QString path = QPEApplication::qpeDir() + "/plugins/inputmethods";
    QDir dir( path, "lib*.so" );
    QStringList list = dir.entryList();
    QStringList::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
	InputMethodInterface *iface = 0;
	QLibrary *lib = new QLibrary( path + "/" + *it );
	if ( lib->queryInterface( IID_InputMethod, (QUnknownInterface**)&iface ) == QS_OK ) {
	    InputMethod input;
	    input.library = lib;
	    input.interface = iface;
	    input.widget = input.interface->inputMethod( 0, inputWidgetStyle );
	    input.interface->onKeyPress( this, SLOT(sendKey(ushort,ushort,ushort)) );
	    inputMethodList.append( input );
	} else {
	    delete lib;
	}
    }
#else
    InputMethod input;
    input.interface = new HandwritingImpl();
    input.widget = input.interface->inputMethod( 0, inputWidgetStyle );
    input.interface->onKeyPress( this, SLOT(sendKey(ushort,ushort,ushort)) );
    inputMethodList.append( input );
    input.interface = new KeyboardImpl();
    input.widget = input.interface->inputMethod( 0, inputWidgetStyle );
    input.interface->onKeyPress( this, SLOT(sendKey(ushort,ushort,ushort)) );
    inputMethodList.append( input );
    input.interface = new PickboardImpl();
    input.widget = input.interface->inputMethod( 0, inputWidgetStyle );
    input.interface->onKeyPress( this, SLOT(sendKey(ushort,ushort,ushort)) );
    inputMethodList.append( input );
#endif
    if ( !inputMethodList.isEmpty() ) {
	method = &inputMethodList[0];
	kbdButton->setPixmap( *method->interface->icon() );
    }
    if ( !inputMethodList.isEmpty() )
	kbdButton->show();
    else
	kbdButton->hide();
    if ( inputMethodList.count() > 1 )
	kbdChoice->show();
    else
	kbdChoice->hide();
}

void InputMethods::chooseKbd()
{
    QPopupMenu pop( this );

    int i = 0;
    QValueList<InputMethod>::Iterator it;
    for ( it = inputMethodList.begin(); it != inputMethodList.end(); ++it, i++ ) {
	pop.insertItem( (*it).interface->name(), i );
	if ( method == &(*it) )
	    pop.setItemChecked( i, TRUE );
    }

    QPoint pt = mapToGlobal(kbdChoice->geometry().topRight());
    QSize s = pop.sizeHint();
    pt.ry() -= s.height();
    pt.rx() -= s.width();
    i = pop.exec( pt );
    if ( i == -1 )
	return;
    InputMethod *im = &inputMethodList[i];
    if ( im != method ) {
	if ( method && method->widget->isVisible() )
	    method->widget->hide();
	method = im;
	kbdButton->setPixmap( *method->interface->icon() );
    }
    if ( !kbdButton->isOn() )
	kbdButton->setOn( TRUE );
    else
	showKbd( TRUE );
}


void InputMethods::showKbd( bool on )
{
    if ( !method )
	return;

    if ( on ) {
	method->interface->resetState();
	int height = method->widget->sizeHint().height();
	method->widget->resize( qApp->desktop()->width(), height );
	method->widget->move( 0, mapToGlobal( QPoint() ).y() - height );
	method->widget->show();
    } else {
	method->widget->hide();
    }

    emit inputToggled( on );
}


void InputMethods::sendKey( ushort unicode, ushort scancode, ushort mod )
{
#if defined(Q_WS_QWS)
    QWSServer::sendKeyEvent( unicode, scancode, mod, TRUE, FALSE );
    QWSServer::sendKeyEvent( unicode, scancode, mod, FALSE, FALSE );
#endif
}
