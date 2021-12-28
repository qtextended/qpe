/****************************************************************************
** $Id: qcopenvelope_qws.cpp,v 1.3 2001/08/30 06:26:22 warwick Exp $
**
** QCOP protocol classes
**
** Created : 20000616
**
** Copyright (C) 2000-2001 Trolltech AS.  All rights reserved.
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

#include "qcopenvelope_qws.h"
#include "global.h"
#include <qbuffer.h>
#include <qdatastream.h>
#include <qfile.h>
#include <unistd.h>

#ifndef QT_NO_COP

/*!
  \class QCopEnvelope qcopenvelope_qws.h
  \brief The QCopEnvelope class encapsulates QCop message sending.

  QCop messages allow applications to communicate with each other.
  These messages are send using QCopEnvelope, and received by connecting
  to a QCopChannel.

  To send a message, use the following protocol:

  \code
     QCopEnvelope e(channelname, messagename);
     e << parameter1 << parameter2 << ...;
  \endcode

  For messages without parameters, you can simply use:

  \code
     QCopEnvelope (channelname, messagename);
  \endcode

  Do not try to simplify further as some compilers may not do
  as you expect.

  The <tt>channelname</tt> of channels within Qt Palmtop all start with "QPE/".
  The <tt>messagename</tt> is a function identifier followed by a list of types
  in parentheses. There are no spaces in the message name.

  To receive a message, you will generally just use your applications
  predefined QPE/Application/<i>appname</i> channel
  (see QPEApplication::appMessage()), but you can make another channel
  and connect it to a slot with:

  \code
      myChannel = new QCopChannel( "QPE/FooBar", this );
      connect( myChannel, SIGNAL(received(const QCString &, const QByteArray &)),
               this, SLOT(fooBarMessage( const QCString &, const QByteArray &)) );
  \endcode

  See also, the \link qcop.html list of Qt Palmtop messages\endlink.
*/

/*!
  Constructs a QCopEnvelope that will write \a message to \a channel.
  If \a message has parameters, you must then user operator<<() to
  write the parameters.
*/
QCopEnvelope::QCopEnvelope( const QCString& channel, const QCString& message ) :
    QDataStream(new QBuffer),
    ch(channel), msg(message)
{
    device()->open(IO_WriteOnly);
}

/*!
  Writes the completed message and destroys the QCopEnvelope.
*/
QCopEnvelope::~QCopEnvelope()
{
    QByteArray data = ((QBuffer*)device())->buffer();
    const int pref=16;
    if ( qstrncmp(ch.data(),"QPE/Application/",pref)==0 ) {
	if ( !QCopChannel::isRegistered(ch) ) {
	    static int seq=1000000;
	    QString qcopfn("/tmp/qcop-msg-");
	    qcopfn += QString::number(seq++);
	    qcopfn += QString::number(getpid());
	    QFile qcopfile(qcopfn);
	    if ( qcopfile.open(IO_WriteOnly) ) {
		{
		    QDataStream ds(&qcopfile);
		    ds << ch << msg << data;
		    qcopfile.close();
		}
		QString cmd = ch.mid(pref);
		cmd += " -qcop " + qcopfn;
		Global::execute(cmd);

		char c;
		for (int i=0; (c=msg[i]); i++) {
		    if ( c == ' ' ) {
			// Return-value required
			// ###### wait for it
			break;
		    } else if ( c == '(' ) {
			// No return value
			break;
		    }
		}
		goto end;
	    }
	}
    }
    QCopChannel::send(ch,msg,data);
end:
    delete device();
}

#endif
