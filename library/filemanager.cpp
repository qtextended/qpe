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
#include "filemanager.h"
#include "applnk.h"

#include <qfile.h>
#include <qdir.h>
#include <qtextstream.h>
#include <qtextcodec.h>

#include <stdlib.h>

/*!
  \class FileManager
  \brief The FileManager class assists with AppLnk input/output.
*/

/*!
  Constructs a FileManager.
*/
FileManager::FileManager()
{
}

/*!
  Destroys a FileManager.
*/
FileManager::~FileManager()
{
    
}

/*!
  Saves \a data as the document specified by \a f.

  Returns whether the operation succeeded.
*/
bool FileManager::saveFile( const DocLnk &f, const QByteArray &data )
{
    QString fn = f.file();
    f.writeLink();
    QFile fl( fn );
    if ( !fl.open( IO_WriteOnly ) )
	return FALSE;
    fl.writeBlock( data );
    fl.close();
    return TRUE;
}

/*!
  Saves \a text as the document specified by \a f.

  The text is saved in UTF8 format.

  Returns whether the operation succeeded.
*/
bool FileManager::saveFile( const DocLnk &f, const QString &text )
{
    QString fn = f.file();
    f.writeLink();
    QFile fl( fn );
    if ( !fl.open( IO_WriteOnly ) )
	return FALSE;
    QTextStream ts( &fl );
#if QT_VERSION <= 230 && defined(QT_NO_CODECS)
    // The below should work, but doesn't in Qt 2.3.0
    ts.setCodec( QTextCodec::codecForMib( 106 ) );
#else
    ts.setEncoding( QTextStream::UnicodeUTF8 );
#endif
    ts << text;
    fl.close();
    return TRUE;
}


/*!
  Loads \a text from the document specified by \a f.

  The text is required to be in UTF8 format.

  Returns whether the operation succeeded.
*/
bool FileManager::loadFile( const DocLnk &f, QString &text )
{
    QString fn = f.file();
    QFile fl( fn );
    if ( !fl.open( IO_ReadOnly ) )
	return FALSE;
    QTextStream ts( &fl );
#if QT_VERSION <= 230 && defined(QT_NO_CODECS)
    // The below should work, but doesn't in Qt 2.3.0
    ts.setCodec( QTextCodec::codecForMib( 106 ) );
#else
    ts.setEncoding( QTextStream::UnicodeUTF8 );
#endif
    text = ts.read();
    fl.close();
    return TRUE;
}


/*!
  Loads \a ba from the document specified by \a f.

  Returns whether the operation succeeded.
*/
bool FileManager::loadFile( const DocLnk &f, QByteArray &ba )
{
    QString fn = f.file();
    QFile fl( fn );
    if ( !fl.open( IO_ReadOnly ) )
	return FALSE;
    ba.resize( fl.size() );
    if ( fl.size() > 0 )
	fl.readBlock( ba.data(), fl.size() );
    fl.close();
    return TRUE;
}

/*!
  Opens the document specified by \a f as a readable QIODevice.
  The caller must delete the return value.

  Returns 0 if the operation fails.
*/
QIODevice* FileManager::openFile( const DocLnk& f )
{
    QString fn = f.file();
    QFile* fl = new QFile( fn );
    if ( !fl->open( IO_ReadOnly ) ) {
	delete fl;
        fl = 0;
    }
    return fl;
}

/*!
  Opens the document specified by \a f as a writable QIODevice.
  The caller must delete the return value.

  Returns 0 if the operation fails.
*/
QIODevice* FileManager::saveFile( const DocLnk& f )
{
    QString fn = f.file();
    f.writeLink();
    QFile* fl = new QFile( fn );
    if ( !fl->open( IO_WriteOnly ) ) {
	delete fl;
        fl = 0;
    }
    return fl;
}

/*!
  Returns whether the document specified by \a f current exists
  as a file on disk.
*/
bool FileManager::exists( const DocLnk &f )
{
    return QFile::exists(f.file());
}


