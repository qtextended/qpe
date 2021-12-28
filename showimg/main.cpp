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
#include "showimg.h"
#include <qpeapplication.h>
#include <qimage.h>

int main( int argc, char **argv )
{
/*
    if ( argc > 1 && QString(argv[1]) == "-m" )
	QPEApplication::setColorSpec( QApplication::ManyColor ), argc--, argv++;
    else if ( argc > 1 && QString(argv[1]) == "-n" )
	QPEApplication::setColorSpec( QApplication::NormalColor ), argc--, argv++;
    else
	QPEApplication::setColorSpec( QApplication::CustomColor );
*/
    QPEApplication a( argc, argv );
    
    ImageViewer w(0, "new window", Qt::WResizeNoErase );
    a.showMainDocumentWidget(&w);

    return a.exec();
}
