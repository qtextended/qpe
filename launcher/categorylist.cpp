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
** $Id: categorylist.cpp,v 1.3 2001/08/29 08:32:15 warwick Exp $
**
**********************************************************************/
#include "categorylist.h"
#include "config.h"
#include <qheader.h>
#include <qdict.h>

class CategoryListPrivate {
public:
    CategoryListPrivate() :
	cfg("CategoryList")
    {
	cfg.setGroup("Categories");
    }

    QDict<QCheckListItem> item;
    Config cfg;
    int ncats;
};

CategoryList::CategoryList( QWidget* parent, const char* name ) :
    QListView(parent,name),
    d(new CategoryListPrivate)
{
    addColumn("");
    header()->hide();

    updateCategories();
}

void CategoryList::addItem(const QString& category)
{
    if ( category.isEmpty() )
	return ;

    QCheckListItem* item = d->item.find(category);
    if ( item ) {
	item->setOn(TRUE);
    } else {
	addItemDirect(category,TRUE);
	d->cfg.writeEntry("Cat"+QString::number(d->ncats++), category);
    }
}

void CategoryList::removeItem(const QString& category)
{
    if ( category.isEmpty() )
	return ;

    QCheckListItem* i = d->item.find(category);
    if ( i ) {
	i->setOn(TRUE);
    } else {
	d->item.remove(category);
	delete i;
	d->cfg.clearGroup();
	QDictIterator<QCheckListItem> it(d->item);
	d->ncats = 0;
	while ( (i=it.current()) ) {
	    ++it;
	    d->cfg.writeEntry("Cat"+QString::number(d->ncats++),i->text());
	}
    }
}

void CategoryList::addItemDirect(const QString& category, bool checked)
{
    QCheckListItem* cb = new QCheckListItem(this,category,QCheckListItem::CheckBox);
    d->item.insert(category,cb);
    cb->setOn(checked);
}

CategoryList::~CategoryList()
{
    delete d;
}

QSize CategoryList::sizeHint() const
{
    return QSize(-1,-1);
}

QSize CategoryList::minimumSizeHint() const
{
    return QSize(-1,-1);
}

QStringList CategoryList::checkedList() const
{
    QStringList checked;
    QCheckListItem* cb=(QCheckListItem*)firstChild();
    for (; cb; cb=(QCheckListItem*)cb->nextSibling()) {
	if ( cb->isOn() )
	    checked.append(cb->text());
    }
    return checked;
}

void  CategoryList::setCheckedList( const QStringList& list )
{
    QCheckListItem* cb=(QCheckListItem*)firstChild();
    for (; cb; cb=(QCheckListItem*)cb->nextSibling())
	cb->setOn(FALSE);
    for ( QStringList::ConstIterator it=list.begin(); it!=list.end(); ++it )
	addItem(*it);
}

void CategoryList::updateCategories()
{
    QDict<void> checked;
    QCheckListItem* cb=(QCheckListItem*)firstChild();
    for (; cb; cb=(QCheckListItem*)cb->nextSibling()) {
	if ( cb->isOn() )
	    checked.insert(cb->text(),(void*)1);
    }
    clear();
    for (d->ncats=0; ; ++d->ncats) {
	QString cat = d->cfg.readEntry("Cat"+QString::number(d->ncats));
	if ( cat.isEmpty() )
	    break;
	if ( d->item.find(cat) ) {
	    qWarning("Duplicate category skipped: %s",cat.latin1());
	} else {
	    addItemDirect(cat,checked.find(cat));
	}
    }
}
