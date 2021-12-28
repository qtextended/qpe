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
** $Id: storage.cpp,v 1.4 2001/09/10 05:15:12 warwick Exp $
**
**********************************************************************/

#include <qlabel.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qlayout.h>
#include "graph.h"
#include "storage.h"


#if defined(_OS_LINUX_) || defined(Q_OS_LINUX)
#include <stdio.h>
#include <sys/vfs.h>
#include <mntent.h>
#endif

StorageInfo::StorageInfo( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    QVBoxLayout *vb = new QVBoxLayout( this );
    MountInfo *mi = new MountInfo( "/", tr("Main storage"), this );
    vb->addWidget( mi );
    QFrame *f = new QFrame( this );
    f->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    vb->addWidget( f );
    mi = new MountInfo( "/mnt/ide", tr("Storage card"), this );
    vb->addWidget( mi );
}

MountInfo::MountInfo( const QString &path, const QString &ttl, QWidget *parent, const char *name )
    : QWidget( parent, name ), title(ttl)
{
    fs = new FileSystem( path );
    QVBoxLayout *vb = new QVBoxLayout( this, 5 );

    totalSize = new QLabel( this );
    vb->addWidget( totalSize );

    data = new GraphData();
    graph = new BarGraph( this );
    graph->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    vb->addWidget( graph, 1 );
    graph->setData( data );

    legend = new GraphLegend( this );
    vb->addWidget( legend );
    legend->setData( data );

    updateData();

    QTimer *t = new QTimer( this );
    connect( t, SIGNAL( timeout() ), this, SLOT( updateData() ) );
    t->start( 5000 );
}

MountInfo::~MountInfo()
{
    delete data;
}

void MountInfo::updateData()
{
    fs->update();
    if ( fs->mounted() ) {
	long mult = fs->blockSize() / 1024;
	long div = 1024 / fs->blockSize();
	if ( !mult ) mult = 1;
	if ( !div ) div = 1;
	long total = fs->totalBlocks() * mult / div;
	long avail = fs->availBlocks() * mult / div;
	long used = total - avail;
	totalSize->setText( title + tr(" Total: %1 kB").arg( total ) );
	data->clear();
	data->addItem( tr("Used (%1 kB)").arg(used), used );
	data->addItem( tr("Available (%1 kB)").arg(avail), avail );
	graph->repaint( FALSE );
	legend->update();
	graph->show();
	legend->show();
    } else {
	totalSize->setText( title + tr(" unavailable") );
	graph->hide();
	legend->hide();
    }
}

//---------------------------------------------------------------------------

FileSystem::FileSystem( const QString &p )
    : fspath( p ), blkSize(512), totalBlks(0), availBlks(0)
{
    update();
}

void FileSystem::update()
{
#if defined(_OS_LINUX_) || defined(Q_OS_LINUX)
    mnted = false;
    struct mntent *me;
    FILE *mntfp = setmntent( "/etc/mtab", "r" );
    if ( mntfp ) {
	while ( (me = getmntent( mntfp )) != 0 ) {
	    if ( me->mnt_dir == fspath ) {
		mnted = true;
		break;
	    }
	}
	endmntent( mntfp );
    }

    if ( mnted ) {
	struct statfs fs;
	if ( !statfs( fspath.latin1(), &fs ) ) {
	    blkSize = fs.f_bsize;
	    totalBlks = fs.f_blocks;
	    availBlks = fs.f_bavail;
	}
    }
#endif
}

