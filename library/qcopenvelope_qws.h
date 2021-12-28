/****************************************************************************
** $Id: qcopenvelope_qws.h,v 1.1 2001/03/07 08:49:53 warwick Exp $
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
#ifndef QCOPENVELOPE_H
#define QCOPENVELOPE_H

#ifndef QT_H
#include <qcopchannel_qws.h>
#include <qdatastream.h>
#endif // QT_H

#ifndef QT_NO_COP

class QCopEnvelope : public QDataStream {
    QCString ch, msg;
public:
    QCopEnvelope( const QCString& channel, const QCString& message );
    ~QCopEnvelope();
};

#endif

#endif
