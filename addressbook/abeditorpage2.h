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
** $Id: abeditorpage2.h,v 1.6 2001/08/27 15:11:56 twschulz Exp $
**
**********************************************************************/

#ifndef _ABEDITORPAGE2_H_
#define _ABEDITORPAGE2_H_


#include <qlist.h>
#include <qstringlist.h>
#include "abeditorpage2base.h"

class AbEditorPage2 : public AbEditorPage2Base
{
    Q_OBJECT
public:
    AbEditorPage2( QWidget *parent = 0, const char *name = 0 );
    ~AbEditorPage2();

    QList<QLineEdit> txts();
    QStringList fields();
    QStringList values();
    void loadFields();

public slots:
    void slotHide();
    void slotShow();

private:
    void init();
    QList<QLabel> listLabels;
    QList<QLineEdit> listTxt;
    QStringList slFields;
};

#endif // _ABEDITORPAGE2_H_
