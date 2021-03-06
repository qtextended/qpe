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

#include "shutdownimpl.h"
#include <global.h>
#include <qtimer.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>

ShutdownImpl::ShutdownImpl( QWidget* parent, const char *name, WFlags fl )
    : Shutdown( parent, name, fl )
{
    timer = new QTimer( this );
    connect( timer, SIGNAL(timeout()), this, SLOT(timeout()) );

    connect( ButtonGroup1, SIGNAL(clicked(int)), this, SLOT(buttonClicked(int)) );
    connect( cancel, SIGNAL(clicked()), this, SLOT(cancelClicked()) );

    progressBar->hide();
    Global::hideInputMethod();
}

void ShutdownImpl::buttonClicked( int b )
{
    progress = 0;
    switch ( b ) {
	case 1:
	    operation = ShutdownSystem;
	    break;
	case 2:
	    operation = RebootSystem;
	    break;
	case 3:
	    operation = RestartDesktop;
	    break;
	case 4:
	    operation = TerminateDesktop;
	    break;
    }
    progressBar->show();
    timer->start( 300 );
    timeout();
}

void ShutdownImpl::cancelClicked()
{
    progressBar->hide();
    if ( timer->isActive() )
	timer->stop();
    else
	close();
}

void ShutdownImpl::timeout()
{
    if ( (progress+=2) > progressBar->totalSteps() ) {
	progressBar->hide();
	timer->stop();
	emit shutdown( operation );
    } else {
	progressBar->setProgress( progress );
    }
}


