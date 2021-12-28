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
#ifndef __INPUT_METHODS_H__
#define __INPUT_METHODS_H__


#include "inputmethodinterface.h"
#include <qwidget.h>
#include <qvaluelist.h>

class QToolButton;
class QLibrary;

struct InputMethod
{
#ifndef QT_NO_COMPONENT
    QLibrary *library;
#endif
    InputMethodInterface *interface;
    QWidget *widget;
};

class InputMethods : public QWidget
{
    Q_OBJECT
public:
    InputMethods( QWidget * );

    QRect inputRect() const;
    void showInputMethod();
    void hideInputMethod();
    void loadInputMethods();

signals:
    void inputToggled( bool on );

private slots:
    void chooseKbd();
    void showKbd( bool );
    void resetStates();
    void sendKey( ushort unicode, ushort scancode, ushort modifiers );

private:
    QToolButton *kbdButton;
    QToolButton *kbdChoice;
    InputMethod *method;
    QValueList<InputMethod> inputMethodList;
};


#endif // __INPUT_METHODS_H__

