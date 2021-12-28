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

#include <qtabwidget.h>
#include <qlayout.h>
#include "memory.h"
#include "load.h"
#include "storage.h"
//#include "graphics.h"
#include "version.h"
#include "sysinfo.h"
#include "resource.h"

SystemInfo::SystemInfo( QWidget *parent, const char *name, WFlags f )
    : QWidget( parent, name, f )
{
    setIcon( Resource::loadPixmap( "system_icon" ) );
    setCaption( tr("System Information") );
    QVBoxLayout *lay = new QVBoxLayout( this );
    QTabWidget *tab = new QTabWidget( this );
    lay->addWidget( tab );
    tab->addTab( new MemoryInfo( tab ), tr("Memory") );
#if defined(_OS_LINUX_) || defined(Q_OS_LINUX)
    tab->addTab( new StorageInfo( tab ), tr("Storage") );
#endif
    tab->addTab( new LoadInfo( tab ), tr("CPU") );
//    tab->addTab( new Graphics( tab ), tr("Graphics") );
    tab->addTab( new VersionInfo( tab ), tr("Version") );

    resize( 220, 180 );
}


