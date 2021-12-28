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
#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <qstring.h>
#include <qstringlist.h>

class DocLnk;
class FileManagerPrivate;

class FileManager
{
public:
    FileManager();
    ~FileManager();

    bool saveFile( const DocLnk&, const QByteArray &data );
    bool saveFile( const DocLnk&, const QString &text );
    bool loadFile( const DocLnk&, QByteArray &data );
    bool loadFile( const DocLnk&, QString &text );

    // The caller must delete the return values.
    QIODevice* openFile( const DocLnk& );
    QIODevice* saveFile( const DocLnk& );

    bool exists( const DocLnk& );
private:
    FileManagerPrivate *d;
};


#endif
