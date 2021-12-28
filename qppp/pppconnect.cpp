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
#include "pppconnect.h"
#include <qfile.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qlineedit.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

static void addOrPick( QComboBox* combo, const QString& t )
{
    for (int i=0; i<combo->count(); i++) {
        if ( combo->text(i) == t ) {
            combo->setCurrentItem(i);
            return;
        }
    }
    combo->setEditText(t);
}

PppConnect::PppConnect() :
    config("PppConnect")
{
    readConfig();
    connect(go,SIGNAL(clicked()),this,SLOT(goGo()));
    findPppPid();
    init();
}

void PppConnect::findPppPid()
{
    QFile pid("/var/run/ppp0.pid");
    oldpid = 0;
    if ( pid.open(IO_ReadOnly))
	oldpid = QString(pid.readAll()).toInt();
}

void PppConnect::init()
{
    if ( oldpid ) {
	go->setText( PppConnectBase::tr("Disconnect") );
	connector->setEnabled(FALSE);
	options->setEnabled(FALSE);
    } else {
	go->setText( PppConnectBase::tr("Connect") );
	connector->setEnabled(TRUE);
	options->setEnabled(TRUE);
    }
}

void PppConnect::done(int ok)
{
    if ( ok )
	writeConfig();
    PppConnectBase::done(ok);
}

void PppConnect::goGo()
{
    if ( oldpid ) {
	findPppPid();
	if ( oldpid )
	    kill( oldpid, SIGINT );
	oldpid = 0;
    } else {
	QString cmd = "pppd /dev/";
	if ( irda->isChecked() )
	    cmd += "ircomm0";
	else if ( cradle->isChecked() )
	    cmd += "ttySA0"; // ### all cradles?
	else
	    cmd += devname->text();
	cmd += " 115200 "; // ### config
	cmd += makeIp(localip->currentText());
	cmd += ":";
	cmd += makeIp(remoteip->currentText());
	cmd += " nocrtscts debug noauth"; // ### config
	cmd += " &";
	system(cmd);
	oldpid = -1; // don't know yet, but suspect it's connected
    }
    init();
}

QString PppConnect::makeIp(const QString& pattern)
{
    QString r = pattern;
    int rnd;
    while ( (rnd=r.find("??")) >= 0 )
	r.replace(rnd,2,QString::number(rand()%253+1)); // 1..254
    return r;
}

static void readComboEntry(Config& config, QComboBox* c, const QString& n)
{
    QStringList l = config.readListEntry(n+"s",',');
    if ( !l.isEmpty() ) {
	c->clear();
	c->insertStringList(l);
    }
    QString i = config.readEntry(n);
    if ( !i.isNull() )
	addOrPick(c,i);
}

static void writeComboEntry(Config& config, const QComboBox* c, const QString& n)
{
    QStringList l;
    for (int i=0; i<c->count(); i++)
	l.append(c->text(i));
    config.writeEntry(n+"s",l,',');
    config.writeEntry(n,c->currentText());
}

void PppConnect::readConfig()
{
    config.setGroup("Config");
    QString dev = config.readEntry("dev","irda");
    if ( dev == "irda" )
	irda->setChecked(TRUE);
    else if ( dev == "cradle" )
	cradle->setChecked(TRUE);
    else
	device->setChecked(TRUE);

    devname->setText(config.readEntry("devname"));

    readComboEntry(config,username,"username");
    readComboEntry(config,localip,"localip");
    readComboEntry(config,remoteip,"remoteip");

    QStringList uns = config.readListEntry("usernames",',');
    config.setGroup("Passwords");
    for (QStringList::ConstIterator it = uns.begin(); it != uns.end(); ++it)
	pw[*it] = config.readEntry(*it);

    password->setText(pw[username->currentText()]);
}

void PppConnect::writeConfig()
{
    config.setGroup("Config");

    QString dev;
    if ( irda->isChecked() )
	dev = "irda";
    else if ( cradle->isChecked() )
	dev = "cradle";
    else
	dev = "other";
    config.writeEntry("dev",dev);
    config.writeEntry("devname",devname->text());

    writeComboEntry(config,username,"username");
    writeComboEntry(config,localip,"localip");
    writeComboEntry(config,remoteip,"remoteip");

    config.setGroup("Passwords");
    config.clearGroup();
    if ( savepassword->isChecked() )
	pw[username->currentText()] = password->text();
    else
	pw.remove(username->currentText());
    QStringList uns = config.readListEntry("usernames",',');
    for (QStringList::ConstIterator it = uns.begin(); it != uns.end(); ++it)
	config.writeEntry(*it, pw[*it]);
}

