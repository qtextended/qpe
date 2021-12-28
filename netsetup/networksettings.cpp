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

#include <stdlib.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qbuffer.h>
#include <qmessagebox.h>
#include <qpeapplication.h>
#include "networksettings.h"

static const char cfgFallback[] = {
"#### DO NOT EDIT Generated by netsetup\n"
"case \"$ADDRESS\" in\n"
"*,*,*,*)\n"
"    INFO=Fallback network setup\n"
"    IF_PORT=\n"
"    BOOTP=n\n"
"    DHCP=y\n"
"    DHCP_HOSTNAME=\n"
"    IPADDR=\n"
"    NETMASK=255.255.255.0\n"
"    NETWORK=\n"
"    GATEWAY=\n"
"    DOMAIN=\n"
"    SEARCH=\n"
"    DNS_1=\n"
"    DNS_2=\n"
"    DNS_3=\n"
"    MOUNTS=\n"
"    MTU=\n"
"    start_fn () { return; }\n"
"    stop_fn () { return; }\n"
"    NO_CHECK=n\n"
"    NO_FUSER=n\n"
"    ;;\n"
"esac\n"
};

NetworkSettings::NetworkSettings()
    : NetworkSettingsBase()
{
    ipGroup = new QButtonGroup( this );
    ipGroup->hide();
    ipGroup->insert( autoIPRadio );
    ipGroup->insert( manualIPRadio );
    connect( ipGroup, SIGNAL(clicked(int)), this, SLOT(ipSelect(int)) );
    ipGroup->setButton( 0 );
    ipFrame->setEnabled( false );

    readConfig();
}

void NetworkSettings::ipSelect( int id )
{
    if ( id == 0 ) {
	ipFrame->setEnabled( false );
	config.replace( "DHCP", "y" );
	gatewayEdit->setEnabled( false );
	dns1Edit->setEnabled( false );
	dns2Edit->setEnabled( false );
    } else {
	ipFrame->setEnabled( true );
	gatewayEdit->setEnabled( true );
	dns1Edit->setEnabled( true );
	dns2Edit->setEnabled( true );
	config.replace( "DHCP", "n" );
    }
}

void NetworkSettings::readConfig()
{
    QByteArray ba;
    ba.setRawData( cfgFallback, sizeof(cfgFallback) );
    QBuffer deflt( ba );
    deflt.open( IO_ReadOnly );
    QIODevice *iodev = &deflt;

//    QFile file( "network.opts" );
    QFile file( "/etc/pcmcia/network.opts" );
    if ( !file.open( IO_ReadOnly ) ) {
	file.setName( QPEApplication::qpeDir() + "etc/network.opts.qpe" );
	file.open( IO_ReadOnly );
    }

    if ( file.isOpen() )
	iodev = &file;

    QTextStream ts( iodev );

    QString line;
    while ( !ts.atEnd() ) {
	line = ts.readLine();
	QString key;
	QString value;
	if ( parseLine( line, key, value ) ) {
	    config[key] = value;
	}
    }

    int id = config["DHCP"] == "y" ? 0 : 1;
    ipGroup->setButton( id );
    ipSelect( id );

    ipEdit->setText( config["IPADDR"] );
    subnetEdit->setText( config["NETMASK"] );
    gatewayEdit->setText( config["GATEWAY"] );
    dns1Edit->setText( config["DNS_1"] );
    dns2Edit->setText( config["DNS_2"] );

    deflt.close();
    ba.resetRawData( cfgFallback, sizeof(cfgFallback) );
}

bool NetworkSettings::writeConfig()
{
    bool ok = false;

    config.replace( "PUMP", config["DHCP"] );
    config.replace( "IPADDR", ipEdit->text() );
    config.replace( "NETMASK", subnetEdit->text() );
    config.replace( "GATEWAY", gatewayEdit->text() );
    config.replace( "DNS_1", dns1Edit->text() );
    config.replace( "DNS_2", dns2Edit->text() );

    // create a broadcast address
    QString bcast = config["IPADDR"];
    int sep = bcast.findRev( "." );
    if ( sep > 4 ) {
	bcast.truncate( sep );
	bcast += ".255";
	config.replace( "BROADCAST", bcast );
    }

    QByteArray ba;
    ba.setRawData( cfgFallback, sizeof(cfgFallback) );
    QBuffer deflt( ba );
    deflt.open( IO_ReadOnly );
    QIODevice *iodev = &deflt;

    QFile tmplFile( QPEApplication::qpeDir() + "etc/network.opts.qpe" );
    if ( tmplFile.open( IO_ReadOnly ) )
	iodev = &tmplFile;

    QTextStream tmplTs( iodev );

//    QFile outFile( "network.opts" );
    QFile outFile( "/etc/pcmcia/network.opts" );
    if ( outFile.open( IO_WriteOnly ) ) {
	QTextStream ts( &outFile );
	QString line;
	while ( !tmplTs.atEnd() ) {
	    line = tmplTs.readLine();
	    QString key;
	    QString value;
	    if ( parseLine( line, key, value ) ) {
		if ( config.contains( key ) ) {
		    QString indent;
		    int i = 0;
		    while ( line[i].isSpace() )
			indent += line[i++];
		    ts << indent << key << "=\"" << config[key] << "\"" << endl;
		} else {
		    ts << line << endl;
		}
	    } else {
		ts << line << endl;
	    }
	}
	ok = true;
    } else {
	QMessageBox::warning( this, tr("Network Setup"),
	    tr("Could not write to:\n%1").arg(outFile.name()) );
    }

    deflt.close();
    ba.resetRawData( cfgFallback, sizeof(cfgFallback) );

    outFile.close();
    system( "cardctl eject && cardctl insert" );

    return ok;
}

bool NetworkSettings::parseLine( QString line, QString &key, QString &value )
{
    bool ok = false;

    line = line.simplifyWhiteSpace();
    if ( line[0] != '#' ) {
	int sep = line.find( '=' );
	if ( sep > 0 ) {
	    key = line.left( sep );
	    if ( !key.contains( QRegExp("[\\s\\[\\]$]") ) ) {
		value = line.mid( sep+1 );
		if ( (value[0] == '\"'  && value[value.length()-1] == '\"') ||
		     (value[0] == '\''  && value[value.length()-1] == '\'') ) {
		    value = value.mid( 1, value.length()-2 );
		}
		ok = true;
	    }
	}
    }

    return ok;
}

void NetworkSettings::accept()
{
    if ( writeConfig() )
	QDialog::accept();
}

