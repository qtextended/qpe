/****************************************************************************
**
** This file is part of Qt Palmtop Environment.
**
** Copyright (C) 2001 Trolltech AS.  All rights reserved.
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

#ifndef FONTMANAGER_H
#define FONTMANAGER_H

#include <qfont.h>

class FontManager
{
public:
    enum Spacing { Fixed, Proportional };
    static bool hasUnicodeFont();
    static QFont unicodeFont( Spacing );
    
};


#endif
