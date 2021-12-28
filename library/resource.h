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
#ifndef PIXMAPLOADER_H
#define PIXMAPLOADER_H

#include <qimage.h>
#include <qbitmap.h>
#include <qstringlist.h>

class Resource
{
public:
    Resource() {}

    static QImage loadImage( const QString &name);

    static QPixmap loadPixmap( const QString &name );
    static QBitmap loadBitmap( const QString &name );
    static QString findPixmap( const QString &name );

    static QString findSound( const QString &name );
    static QStringList allSounds();
};

#endif
