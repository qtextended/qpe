/**********************************************************************
** Copyright (C) 2001 Trolltech AS.  All rights reserved.
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
#ifndef FONTFACTORY_H
#define FONTFACTORY_H

#include <qfontdatabase.h>
#include <fontfactoryinterface.h>
#include <qvaluelist.h>

class QLibrary;

struct FontFactory
{
#ifndef QT_NO_COMPONENT
    QLibrary *library;
#endif
    FontFactoryInterface *interface;
    QFontFactory *factory;
};

class FontDatabase
#ifndef QT_NO_FONTDATABASE
    : public QFontDatabase
#endif
{
public:
    FontDatabase();

    // minimal functionality
    QStringList families () const;
#ifdef QT_NO_FONTDATABASE
    static QValueList<int> standardSizes ();
#endif

    static void loadRenderers();

private:
    static void readFonts( QFontFactory *factory );

private:
    static QValueList<FontFactory> *factoryList;
};

#endif
