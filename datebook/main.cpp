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
** $Id: main.cpp,v 1.8 2001/08/27 19:02:23 lknoll Exp $
**
**********************************************************************/
#include <qpeapplication.h>
#include <qpedebug.h>
#include "datebook.h"


int main( int argc, char **argv )
{
    QPEApplication a( argc, argv );

    DateBook e;
    e.setCaption( DateBook::tr("Calendar") );
    a.showMainWidget(&e);
    if ( e.needGui() )
	return a.exec();
    else
	return 0;
}
