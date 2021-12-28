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
** $Id: lnkproperties.cpp,v 1.8 2001/09/07 09:20:31 jryland Exp $
**
**********************************************************************/
#include "lnkproperties.h"
#include "applnk.h"
#include "global.h"
#include "qcopenvelope_qws.h"
#include "filemanager.h"
#include "config.h"
#include "categorylist.h"
#include <qlineedit.h>
#include <qtoolbutton.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qfile.h>

LnkProperties::LnkProperties( AppLnk* l, QWidget* parent )
    : LnkPropertiesBase( parent, 0, TRUE ),
	lnk(l)
{
    docname->setText(l->name());
    QString inf;
    if ( !l->type().isEmpty() ) inf += "<tr><td>Type:<td>" + l->type();
    // comment could be editable here
    if ( !l->comment().isEmpty() ) inf += "<tr><td>Comment:<td>" + l->comment();
    if ( !inf.isEmpty() ) inf = "<table>" + inf + "</table>";
    info->setText(inf);
    connect(open,SIGNAL(clicked()),this,SLOT(openLnk()));
    if ( lnk->type().contains('/') ) { // A document? (#### better predicate needed)
	connect(unlink,SIGNAL(clicked()),this,SLOT(unlinkLnk()));
	connect(duplicate,SIGNAL(clicked()),this,SLOT(duplicateLnk()));

	preload->hide();
	spacer->hide();
	categories->setCheckedList(lnk->categories());
	connect(addcat,SIGNAL(clicked()),this,SLOT(addCat()));
	connect(delcat,SIGNAL(clicked()),this,SLOT(delCat()));
	connect(categories,SIGNAL(clicked(QListViewItem*)),this,SLOT(selCat(QListViewItem*)));
    } else {
	unlink->hide();
	duplicate->hide();

	// Can't edit categories, since the app .desktop files are global,
	// possibly read-only.
	categoriesgrp->hide();

	Config cfg("Launcher");
	cfg.setGroup("Preload");
	QStringList apps = cfg.readListEntry("Apps",',');
	preload->setChecked( apps.contains(l->exec()) );
    }
}

LnkProperties::~LnkProperties()
{
}

void LnkProperties::selCat(QListViewItem* i)
{
    if (i)
        cat->setText(i->text(0));
}

void LnkProperties::addCat()
{
    categories->addItem(cat->text());
}

void LnkProperties::delCat()
{
    categories->removeItem(cat->text());
}

void LnkProperties::openLnk()
{
    accept(); // commit any changes

    lnk->execute();
}

void LnkProperties::unlinkLnk()
{
    // Need some safety here. eg. only delete ~/Documents, keep backup, etc.
    lnk->removeFiles();

    reject();
}

void LnkProperties::duplicateLnk()
{
    // The duplicate takes the new properties.
    DocLnk newdoc;
    newdoc.setType(lnk->type());
    if ( docname->text() == lnk->name() )
	newdoc.setName(tr("Copy of ")+docname->text());
    else
	newdoc.setName(docname->text());
    newdoc.setCategories(lnk->categories());
    FileManager fm;
    QByteArray ba;
    if ( fm.loadFile((DocLnk&)*lnk, ba) )
	fm.saveFile(newdoc, ba);
    reject();
}

void LnkProperties::done(int ok)
{
    if ( ok ) {
	bool changed=FALSE;
	if ( lnk->name() != docname->text() ) {
	    lnk->setName(docname->text());
	    changed=TRUE;
	}
	if ( lnk->categories() != categories->checkedList() ) {
	    lnk->setCategories(categories->checkedList());
	    changed=TRUE;
	}
	if ( changed )
	    lnk->writeLink();

	Config cfg("Launcher");
	cfg.setGroup("Preload");
	QStringList apps = cfg.readListEntry("Apps",',');
	QString exe = lnk->exec();
	if ( apps.contains(exe) != preload->isChecked() ) {
	    if ( preload->isChecked() ) {
		apps.append(exe);
		Global::execute(exe+" -preload");
	    } else {
		apps.remove(exe);
		QCopEnvelope e("QPE/Application/"+exe.local8Bit(),
		    "quitIfInvisible()");
	    }
	    cfg.writeEntry("Apps",apps,',');
	}
    }
    LnkPropertiesBase::done(ok);
}
