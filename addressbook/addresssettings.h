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
** $Id: addresssettings.h,v 1.2 2001/08/26 18:46:33 twschulz Exp $
**
**********************************************************************/

#ifndef _ADDRESSSETTINGS_H_
#define _ADDRESSSETTINGS_H_

#include <qlist.h>
#include <qstringlist.h>
#include "addresssettingsbase.h"

class AddressSettings : public AddressSettingsBase
{
    Q_OBJECT
public:
    AddressSettings( QWidget *parent = 0, const char *name = 0 );
    ~AddressSettings();
    QStringList categories();

protected:
    void accept();

public slots:
    void slotMakeAllUnique();

private:
    void init();
    void save();
    QList<QComboBox> listItems;
};

#endif  // _ADDRESSSETTINGS_H_
