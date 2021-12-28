/****************************************************************************
** $Id: qpeapplication.h,v 1.29 2001/08/27 20:52:29 lknoll Exp $
**
** Implementation of Qt/Embedded Palmtop Environment Application Framework
**
** Created : 000101
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef __QPE_APPLICATION_H__
#define __QPE_APPLICATION_H__


#include <qapplication.h>
#if defined(_WS_QWS_) && !defined(Q_WS_QWS)
#define Q_WS_QWS
#endif
#include "qpedecoration_qws.h"

class QCopChannel;
class QPEApplicationData;
class QWSEvent;

class QPEApplication : public QApplication
{
    Q_OBJECT
public:
    QPEApplication( int& argc, char **argv, Type=GuiClient );
    ~QPEApplication();

    static QString qpeDir();
    static QString documentDir();
    void applyStyle();
    static int defaultRotation();
    static void setDefaultRotation(int r);
    static void grabKeyboard();
    static void ungrabKeyboard();

    enum StylusMode {
	LeftOnly,
	RightOnHold
	// RightOnHoldLeftDelayed, etc.
    };
    static void setStylusOperation( QWidget*, StylusMode );
    static StylusMode stylusOperation( QWidget* );

    enum InputMethodHint {
	Normal,
	AlwaysOff,
	AlwaysOn
    };
    static void setInputMethodHint( QWidget *, InputMethodHint );
    static InputMethodHint inputMethodHint( QWidget * );

    void showMainWidget( QWidget*, bool nomax=FALSE );
    void showMainDocumentWidget( QWidget*, bool nomax=FALSE );

    int exec();

signals:
    void clientMoused();
    void timeChanged();
    void clockChanged( bool pm );
    void appMessage( const QCString& msg, const QByteArray& data);

private slots:
    void systemMessage( const QCString &msg, const QByteArray &data );
    void pidMessage( const QCString &msg, const QByteArray &data );
    void removeSenderFromStylusDict();

protected:
    bool qwsEventFilter( QWSEvent * );
    void internalSetStyle( const QString &style );
    void prepareForTermination(bool willrestart);
    virtual void restart();
    virtual void shutdown();
    bool eventFilter( QObject *, QEvent * );
    void timerEvent( QTimerEvent * );
    bool keyboardGrabbed() const;
    bool raiseAppropriateWindow();

private:
#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
    QCopChannel *sysChannel;
    QCopChannel *pidChannel;
#endif
    QPEApplicationData *d;
};


#endif

