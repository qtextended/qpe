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
** $Id: datebookweekheaderimpl.h,v 1.2 2001/03/28 06:02:56 twschulz Exp $
**
**********************************************************************/
#ifndef DATEBOOKDAYHEADER_H
#define DATEBOOKDAYHEADER_H
#include "datebookweekheader.h"

class DateBookWeekHeader : public DateBookWeekHeaderBase
{
    Q_OBJECT

public:
    DateBookWeekHeader( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~DateBookWeekHeader();

    void setDate( int y, int w );
    
signals:
    void dateChanged( int y, int w );
    
public slots:
    void yearChanged( int );
    void nextWeek();
    void prevWeek();
    void weekChanged( int );

private:
    int year, week;
    
};

#endif // DATEBOOKDAYHEADER_H
