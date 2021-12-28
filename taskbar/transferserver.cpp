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
** $Id: transferserver.cpp,v 1.5 2001/08/31 00:18:53 lknoll Exp $
**
**********************************************************************/
#define _XOPEN_SOURCE
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <shadow.h>

#include <qdir.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qdatastream.h>
#include <qstringlist.h>
#include <qfileinfo.h>
#include <qregexp.h>
#include <qcopchannel_qws.h>

#include "transferserver.h"

const int block_size = 51200;

TransferServer::TransferServer( Q_UINT16 port, QObject *parent = 0, const char* name = 0)
    : QServerSocket( port, 1, parent, name )
{
    if ( !ok() )
	qWarning( "Failed to bind to port %d", port );
}

TransferServer::~TransferServer()
{

}

void TransferServer::newConnection( int socket )
{
    (void) new ServerPI( socket, this );
}

ServerPI::ServerPI( int socket, QObject *parent = 0, const char* name = 0)
    : QSocket( parent, name ) , dtp( 0 ), serversocket( 0 ), waitsocket( 0 )
{
    state = Connected;
    connect( this, SIGNAL( readyRead() ), SLOT( read() ) );
    connect( this, SIGNAL( connectionClosed() ), SLOT( connectionClosed() ) );

    setSocket( socket );

    peerport = peerPort();
    peeraddress = peerAddress();
    passiv = FALSE;
    for( int i = 0; i < 4; i++ )
	wait[i] = FALSE;

    send( "220 Qt Palmtop transfer service ready!" );
    state = Wait_USER;

    dtp = new ServerDTP( this );
    connect( dtp, SIGNAL( completed() ), SLOT( dtpCompleted() ) );
    connect( dtp, SIGNAL( failed() ), SLOT( dtpFailed() ) );
    connect( dtp, SIGNAL( error( int ) ), SLOT( dtpError( int ) ) );


    directory = QDir::currentDirPath();

    static int p = 1024;

    while ( !serversocket || !serversocket->ok() ) {
	delete serversocket;
	serversocket = new ServerSocket( ++p, this );
    }
    connect( serversocket, SIGNAL( newIncomming( int ) ), SLOT( newConnection( int ) ) );
}

ServerPI::~ServerPI()
{

}

void ServerPI::connectionClosed()
{
    // qDebug( "Debug: Connection closed" );
    delete this;
}

void ServerPI::send( const QString& msg )
{
    QTextStream os( this );
    os << msg << endl;
    //qDebug( "Reply: %s", msg.latin1() );
}

void ServerPI::read()
{
    while ( canReadLine() )
	process( readLine().stripWhiteSpace() );
}

bool ServerPI::checkUser( const QString& user )
{
    if ( user.isEmpty() ) return FALSE;

    struct passwd *pw;
    pw = getpwuid( geteuid() );
    QString euser = QString::fromLocal8Bit( pw->pw_name );
    return user == euser;
}

bool ServerPI::checkPassword( const QString& password )
{
    // ### HACK for testing on local host
    return true;

    /*
    struct passwd *pw = 0;
    struct spwd *spw = 0;

    pw = getpwuid( geteuid() );
    spw = getspnam( pw->pw_name );

    QString cpwd = QString::fromLocal8Bit( pw->pw_passwd );
    if ( cpwd == "x" && spw )
	cpwd = QString::fromLocal8Bit( spw->sp_pwdp );

    QString cpassword = QString::fromLocal8Bit( crypt( password.local8Bit(), cpwd.local8Bit() ) );
    return cpwd == cpassword;
*/
}

bool ServerPI::checkReadFile( const QString& file )
{
    QString filename;

    if ( file[0] != "/" )
	filename = directory.path() + "/" + file;
    else
	filename = file;

    QFileInfo fi( filename );
    return ( fi.exists() && fi.isReadable() );
}

bool ServerPI::checkWriteFile( const QString& file )
{
    QString filename;

    if ( file[0] != "/" )
	filename = directory.path() + "/" + file;
    else
	filename = file;

    QFileInfo fi( filename );

    if ( fi.exists() )
	if ( !QFile( filename ).remove() )
	    return FALSE;
    return TRUE;
}

void ServerPI::process( const QString& message )
{
    // qDebug( "Command: %s", message.latin1() );

    // split message using "," as separator
    QStringList msg = QStringList::split( " ", message );
    if ( msg.isEmpty() ) return;

    // command token
    QString cmd = msg[0].upper();

    // argument token
    QString arg;
    if ( msg.count() >= 2 )
	arg = msg[1];


    // we always respond to QUIT, regardless of state
    if ( cmd == "QUIT" ) {
	send( "211 Good bye!" );
	delete this;
	return;
    }

    // connected to client
    if ( Connected == state )
	return;

    // waiting for user name
    if ( Wait_USER == state ) {

	if ( cmd != "USER" || msg.count() < 2 || !checkUser( arg ) ) {
	    send( "530 Please login with USER and PASS" );
	    return;
	}
	send( "331 User name ok, need password" );
	state = Wait_PASS;
	return;
    }

    // waiting for password
    if ( Wait_PASS == state ) {

	if ( cmd != "PASS" || !checkPassword( arg ) ) {
	//if ( cmd != "PASS" || msg.count() < 2 || !checkPassword( arg ) ) {
	    send( "530 Please login with USER and PASS" );
	    return;
	}
	send( "230 User logged in, proceed" );
	state = Ready;
	return;
    }

    // ACCESS CONTROL COMMANDS


    // account (ACCT)
    if ( cmd == "ACCT" ) {
	// even wu-ftp does not support it
	send( "502 Command not implemented" );
    }

    // change working directory (CWD)
    else if ( cmd == "CWD" ) {

	if ( !arg.isEmpty() ) {
	    if ( directory.cd( arg, TRUE ) )
		send( "250 Requested file action okay, completed" );
	    else
		send( "550 Requested action not taken" );
	}
	else
	    send( "500 Syntax error, command unrecognized" );
    }

    // change to parent directory (CDUP)
    else if ( cmd == "CDUP" ) {
	if ( directory.cdUp() )
	    send( "250 Requested file action okay, completed" );
	else
	    send( "550 Requested action not taken" );
    }

    // structure mount (SMNT)
    else if ( cmd == "SMNT" ) {
	// even wu-ftp does not support it
	send( "502 Command not implemented" );
    }

    // reinitialize (REIN)
    else if ( cmd == "REIN" ) {
	// even wu-ftp does not support it
	send( "502 Command not implemented" );
    }


    // TRANSFER PARAMETER COMMANDS


    // data port (PORT)
    else if ( cmd == "PORT" ) {
	if ( parsePort( arg ) )
	    send( "200 Command okay" );
	else
	    send( "500 Syntax error, command unrecognized" );
    }

    // passive (PASV)
    else if ( cmd == "PASV" ) {
	passiv = TRUE;
	send( "227 Entering Passive Mode ("
	      + address().toString().replace( QRegExp( "\\." ), "," ) + ","
	      + QString::number( ( serversocket->port() ) >> 8 ) + ","
	      + QString::number( ( serversocket->port() ) & 0xFF ) +")" );
    }

    // representation type (TYPE)
    else if ( cmd == "TYPE" ) {
	if ( arg.upper() == "A" || arg.upper() == "I" )
	    send( "200 Command okay" );
	else
	    send( "504 Command not implemented for that parameter" );
    }

    // file structure (STRU)
    else if ( cmd == "STRU" ) {
	if ( arg.upper() == "F" )
	    send( "200 Command okay" );
	else
	    send( "504 Command not implemented for that parameter" );
    }

    // transfer mode (MODE)
    else if ( cmd == "MODE" ) {
	if ( arg.upper() == "S" )
	    send( "200 Command okay" );
	else
	    send( "504 Command not implemented for that parameter" );
    }


    // FTP SERVICE COMMANDS


    // retrieve (RETR)
    else if ( cmd == "RETR" )
	if ( !arg.isEmpty() && checkReadFile( absFilePath( arg ) ) ) {
	    send( "150 File status okay" );
	    sendFile( absFilePath( arg ) );
	}
	else
	    send( "550 Requested action not taken" );

    // store (STOR)
    else if ( cmd == "STOR" )
	if ( !arg.isEmpty() && checkWriteFile( absFilePath( arg ) ) ) {
	    send( "150 File status okay" );
	    retrieveFile( absFilePath( arg ) );
	}
	else
	    send( "550 Requested action not taken" );

    // store unique (STOU)
    else if ( cmd == "STOU" ) {
	send( "502 Command not implemented" );
    }

    // append (APPE)
    else if ( cmd == "APPE" ) {
	send( "502 Command not implemented" );
    }

    // allocate (ALLO)
    else if ( cmd == "ALLO" ) {
	send( "200 Command okay" );
    }

    // restart (REST)
    else if ( cmd == "REST" ) {
	send( "502 Command not implemented" );
    }

    // rename from (RNFR)
    else if ( cmd == "RNFR" ) {
	renameFrom = QString::null;
	if ( arg.isEmpty() )
	    send( "500 Syntax error, command unrecognized" );
	else {
	    QFile file( absFilePath( arg ) );
	    if ( file.exists() ) {
		send( "350 File exists, ready for destination name" );
		renameFrom = absFilePath( arg );
	    }
	    else
		send( "550 Requested action not taken" );
	}
    }

    // rename to (RNTO)
    else if ( cmd == "RNTO" ) {
	if ( lastCommand != "RNFR" )
	    send( "503 Bad sequence of commands" );
	else if ( arg.isEmpty() )
	    send( "500 Syntax error, command unrecognized" );
	else {
	    QDir dir( absFilePath( arg ) );
	    if ( dir.rename( renameFrom, absFilePath( arg ), TRUE ) )
		send( "250 Requested file action okay, completed." );
	    else
		send( "550 Requested action not taken" );
	}
    }

    // abort (ABOR)
    else if ( cmd.contains( "ABOR" ) ) {
	dtp->close();
	if ( dtp->dtpMode() != ServerDTP::Idle )
	    send( "426 Connection closed; transfer aborted" );
	else
	    send( "226 Closing data connection" );
    }

    // delete (DELE)
    else if ( cmd == "DELE" ) {
	if ( arg.isEmpty() )
	    send( "500 Syntax error, command unrecognized" );
	else {
	    QFile file( absFilePath( arg ) ) ;
	    if ( file.remove() )
		send( "250 Requested file action okay, completed" );
	    else
		send( "550 Requested action not taken" );
	}
    }

    // remove directory (RMD)
    else if ( cmd == "RMD" ) {
	if ( arg.isEmpty() )
	    send( "500 Syntax error, command unrecognized" );
	else {
	    QDir dir;
	    if ( dir.rmdir( absFilePath( arg ), TRUE ) )
		send( "250 Requested file action okay, completed" );
	    else
		send( "550 Requested action not taken" );
	}
    }

    // make directory (MKD)
    else if ( cmd == "MKD" ) {
	//qDebug("  MKD");
	if ( arg.isEmpty() ) {
	    qDebug(" Error: no arg");
	    send( "500 Syntax error, command unrecognized" );
	}
	else {
	    QDir dir;
	    if ( dir.mkdir( absFilePath( arg ), TRUE ) ) {
		//qDebug("  mkdir %s ok", arg.latin1());
		send( "250 Requested file action okay, completed." );
	    }
	    else {
		//qDebug("  mkdir %s error", arg.latin1() );
		send( "550 Requested action not taken" );
	    }
	}
    }

    // print working directory (PWD)
    else if ( cmd == "PWD" ) {
	send( "257 \"" + directory.path() +"\"" );
    }

    // list (LIST)
    else if ( cmd == "LIST" ) {
	if ( sendList( absFilePath( arg ) ) )
	    send( "150 File status okay" );
	else
	    send( "500 Syntax error, command unrecognized" );
    }

    // size (SIZE)
    else if ( cmd == "SIZE" ) {
	QFileInfo fi( absFilePath( arg ) );
	if ( !fi.exists() )
	    send( "500 Syntax error, command unrecognized" );
	else
	    send( "213 " + QString::number( fi.size() ) );
    }

    // name list (NLST)
    else if ( cmd == "NLST" ) {
	send( "502 Command not implemented" );
    }

    // site parameters (SITE)
    else if ( cmd == "SITE" ) {
	send( "502 Command not implemented" );
    }

    // system (SYST)
    else if ( cmd == "SYST" ) {
	send( "215 UNIX Type: L8" );
    }

    // status (STAT)
    else if ( cmd == "STAT" ) {
	send( "502 Command not implemented" );
    }

    // help (HELP )
    else if ( cmd == "HELP" ) {
	send( "502 Command not implemented" );
    }

    // noop (NOOP)
    else if ( cmd == "NOOP" ) {
	send( "200 Command okay" );
    }

    // call (CALL)
    else if ( cmd == "CALL" ) {

	// example: call QPE/System execute(QString) addressbook

	if ( msg.count() < 3 ) {
	    send( "500 Syntax error, command unrecognized" );
	}
	else {

	    QString channel = msg[1];
	    QString command = msg[2];
	    command.stripWhiteSpace();

	    int paren = command.find( "(" );
	    if ( paren <= 0 ) {
		send( "500 Syntax error, command unrecognized" );
		return;
	    }

	    QString params = command.mid( paren + 1 );
	    if ( params[params.length()-1] != ')' ) {
		send( "500 Syntax error, command unrecognized" );
		return;
	    }

	    params.truncate( params.length()-1 );
	    QByteArray buffer;
	    QDataStream ds( buffer, IO_WriteOnly );

	    int msgId = 3;

	    QStringList paramList = QStringList::split( ",", params );
	    if ( paramList.count() > msg.count() - 3 ) {
		send( "500 Syntax error, command unrecognized" );
		return;
	    }

	    for ( QStringList::Iterator it = paramList.begin(); it != paramList.end(); ++it ) {

		QString arg = msg[msgId];
		if ( *it == "QString" )
		    ds << arg;
		else if ( *it == "QCString" )
		    ds << arg.local8Bit();
		else if ( *it == "int" )
		    ds << arg.toInt();
		else if ( *it == "bool" )
		    ds << arg.toInt();
		else {
		    send( "500 Syntax error, command unrecognized" );
		    return;
		}
		msgId++;
	    }

	    if ( paramList.count() )
		QCopChannel::send( channel.latin1(), command.latin1(), buffer );
	    else
		QCopChannel::send( channel.latin1(), command.latin1() );

	    send( "200 Command okay" );
	}
    }

    // not implemented
    else
	send( "502 Command not implemented" );

    lastCommand = cmd;
}

void ServerPI::sendFile( const QString& file )
{
    if ( passiv ) {
	wait[SendFile] = TRUE;
	waitfile = file;
	if ( waitsocket )
	    newConnection( waitsocket );
    }
    else
	dtp->sendFile( file, peeraddress, peerport );
}

void ServerPI::retrieveFile( const QString& file )
{
    if ( passiv ) {
	wait[RetrieveFile] = TRUE;
	waitfile = file;
	if ( waitsocket )
	    newConnection( waitsocket );
    }
    else
	dtp->retrieveFile( file, peeraddress, peerport );
}

bool ServerPI::parsePort( const QString& pp )
{
    QStringList p = QStringList::split( ",", pp );
    if ( p.count() != 6 ) return FALSE;

    // h1,h2,h3,h4,p1,p2
    peeraddress = QHostAddress( ( p[0].toInt() << 24 ) + ( p[1].toInt() << 16 ) +
				( p[2].toInt() << 8 ) + p[3].toInt() );
    peerport = ( p[4].toInt() << 8 ) + p[5].toInt();
    return TRUE;
}

void ServerPI::dtpCompleted()
{
    dtp->close();
    waitsocket = 0;
    send( "226 Closing data connection, file transfer successful" );
}

void ServerPI::dtpFailed()
{
    dtp->close();
    waitsocket = 0;
    send( "451 Requested action aborted: local error in processing" );
}

void ServerPI::dtpError( int )
{
    dtp->close();
    waitsocket = 0;
    send( "451 Requested action aborted: local error in processing" );
}

bool ServerPI::sendList( const QString& arg )
{
    QByteArray listing;
    QBuffer buffer( listing );

    if ( !buffer.open( IO_WriteOnly ) )
	return FALSE;

    QTextStream ts( &buffer );
    QString fn = arg;

    if ( fn.isEmpty() )
	fn = directory.path();

    QFileInfo fi( fn );
    if ( !fi.exists() ) return FALSE;

    // return file listing
    if ( fi.isFile() ) {
	ts << fileListing( &fi ) << endl;
    }

    // return directory listing
    else if ( fi.isDir() ) {
	QDir dir( fn );
	const QFileInfoList *list = dir.entryInfoList( QDir::All | QDir::Hidden );

	QFileInfoListIterator it( *list );
	QFileInfo *info;

	unsigned long total = 0;
	while ( ( info = it.current() ) ) {
	    if ( info->fileName() != "." && info->fileName() != ".." )
		total += info->size();
	    ++it;
	}

	ts << "total " << QString::number( total / 1024 ) << endl;

	it.toFirst();
	while ( ( info = it.current() ) ) {
	    if ( info->fileName() == "." || info->fileName() == ".." ) {
		++it;
		continue;
	    }
	    ts << fileListing( info ) << endl;
	    ++it;
	}
    }

    if ( passiv ) {
	waitarray = buffer.buffer();
	wait[SendByteArray] = TRUE;
	if ( waitsocket )
	    newConnection( waitsocket );
    }
    else
	dtp->sendByteArray( buffer.buffer(), peeraddress, peerport );
    return TRUE;
}

QString ServerPI::fileListing( QFileInfo *info )
{
    if ( !info ) return QString::null;
    QString s;

    // type char
    if ( info->isDir() )
	s += "d";
    else if ( info->isSymLink() )
	s += "l";
    else
	s += "-";

    // permisson string
    s += permissionString( info ) + " ";

    // number of hardlinks
    int subdirs = 1;

    if ( info->isDir() )
	subdirs = 2;
    // FIXME : this is to slow
    //if ( info->isDir() )
    //subdirs = QDir( info->absFilePath() ).entryList( QDir::Dirs ).count();

    s += QString::number( subdirs ).rightJustify( 3, ' ', TRUE ) + " ";

    // owner
    s += info->owner().leftJustify( 8, ' ', TRUE ) + " ";

    // group
    s += info->group().leftJustify( 8, ' ', TRUE ) + " ";

    // file size in bytes
    s += QString::number( info->size() ).rightJustify( 9, ' ', TRUE ) + " ";

    // last modified date
    QDate date = info->lastModified().date();
    QTime time = info->lastModified().time();
    s += date.monthName( date.month() ) + " "
	 + QString::number( date.day() ).rightJustify( 2, ' ', TRUE ) + " "
	 + QString::number( time.hour() ).rightJustify( 2, '0', TRUE ) + ":"
	 + QString::number( time.minute() ).rightJustify( 2,'0', TRUE ) + " ";

    // file name
    s += info->fileName();

    return s;
}

QString ServerPI::permissionString( QFileInfo *info )
{
    if ( !info ) return QString( "---------" );
    QString s;

    // user
    if ( info->permission( QFileInfo::ReadUser ) ) s += "r";
    else s += "-";
    if ( info->permission( QFileInfo::WriteUser ) ) s += "w";
    else s += "-";
    if ( info->permission( QFileInfo::ExeUser ) ) s += "x";
    else s += "-";

    // group
    if ( info->permission( QFileInfo::ReadGroup ) ) s += "r";
    else s += "-";
    if ( info->permission( QFileInfo::WriteGroup ) )s += "w";
    else s += "-";
    if ( info->permission( QFileInfo::ExeGroup ) ) s += "x";
    else s += "-";

    // exec
    if ( info->permission( QFileInfo::ReadOther ) ) s += "r";
    else s += "-";
    if ( info->permission( QFileInfo::WriteOther ) ) s += "w";
    else s += "-";
    if ( info->permission( QFileInfo::ExeOther ) ) s += "x";
    else s += "-";

    return s;
}

void ServerPI::newConnection( int socket )
{
    // qDebug( "New incomming connection" );

    if ( !passiv ) return;

    if ( wait[SendFile] ) {
	dtp->sendFile( waitfile );
	dtp->setSocket( socket );
    }
    else if ( wait[RetrieveFile] ) {
	dtp->retrieveFile( waitfile );
	dtp->setSocket( socket );
    }
    else if ( wait[SendByteArray] ) {
	dtp->sendByteArray( waitarray );
	dtp->setSocket( socket );
    }
    else if ( wait[RetrieveByteArray] ) {
	dtp->retrieveByteArray();
	dtp->setSocket( socket );
    }
    else
	waitsocket = socket;

    for( int i = 0; i < 4; i++ )
	wait[i] = FALSE;
}

QString ServerPI::absFilePath( const QString& file )
{
    if ( file.isEmpty() ) return file;

    QString filepath( file );
    if ( file[0] != "/" )
	filepath = directory.path() + "/" + file;

    return filepath;
}

ServerDTP::ServerDTP( QObject *parent = 0, const char* name = 0)
    : QSocket( parent, name ), mode( Idle )
{
    connect( this, SIGNAL( connected() ), SLOT( connected() ) );
    connect( this, SIGNAL( connectionClosed() ), SLOT( connectionClosed() ) );
    connect( this, SIGNAL( bytesWritten( int ) ), SLOT( bytesWritten( int ) ) );
    connect( this, SIGNAL( readyRead() ), SLOT( readyRead() ) );
}

ServerDTP::~ServerDTP()
{
    buf.close();
    file.close();
}

void ServerDTP::connected()
{
    // send file mode
    if ( SendFile == mode ) {

	if ( !file.exists() || !file.open( IO_ReadOnly) ) {
	    emit failed();
	    mode = Idle;
	    return;
	}

	// qDebug( "Debug: Sending file %s", file.name().latin1() );

	bytes_written = 0;

	if( !file.atEnd() ) {
	    QCString s;
	    s.resize( block_size );
	    int bytes = file.readBlock( s.data(), block_size );
	    writeBlock( s.data(), bytes );
	}
    }

    // send buffer mode
    else if ( SendBuffer == mode ) {

	if ( !buf.open( IO_ReadOnly) ) {
	    emit failed();
	    mode = Idle;
	    return;
	}

	// qDebug( "Debug: Sending byte array" );

	bytes_written = 0;

	while( !buf.atEnd() )
	    putch( buf.getch() );
	buf.close();
    }

    // retrieve file mode
    else if ( RetrieveFile == mode ) {

	if ( file.exists() && !file.remove() ) {
	    emit failed();
	    mode = Idle;
	    return;
	}

	if ( !file.open( IO_WriteOnly) ) {
	    emit failed();
	    mode = Idle;
	    return;
	}
	// qDebug( "Debug: Retrieving file %s", file.name().latin1() );
    }

    // retrieve buffer mode
    else if ( RetrieveBuffer == mode ) {

	if ( !buf.open( IO_WriteOnly) ) {
	    emit failed();
	    mode = Idle;
	    return;
	}

	// qDebug( "Debug: Retrieving byte array" );
    }
}

void ServerDTP::connectionClosed()
{
    // qDebug( "Debug: Data connection closed %ld bytes written", bytes_written );

    // send file mode
    if ( SendFile == mode ) {
	if ( bytes_written == file.size() )
	    emit completed();
	else
	    emit failed();
    }

    // send buffer mode
    else if ( SendBuffer == mode ) {
	if ( bytes_written == buf.size() )
	    emit completed();
	else
	    emit failed();
    }

    // retrieve file mode
    else if ( RetrieveFile == mode ) {
	file.close();
	emit completed();
    }

    // retrieve buffer mode
    else if ( RetrieveBuffer == mode ) {
	buf.close();
	emit completed();
    }

    mode = Idle;
}

void ServerDTP::bytesWritten( int bytes )
{
    bytes_written += bytes;

    // send file mode
    if ( SendFile == mode ) {

	if ( bytes_written == file.size() ) {
	    // qDebug( "Debug: Sending complete: %d bytes", file.size() );
	    file.close();
	    emit completed();
	    mode = Idle;
	}
	else if( !file.atEnd() ) {
	    QCString s;
	    s.resize( block_size );
	    int bytes = file.readBlock( s.data(), block_size );
	    writeBlock( s.data(), bytes );
	}
    }

    // send buffer mode
    if ( SendBuffer == mode ) {

	if ( bytes_written == buf.size() ) {
	    // qDebug( "Debug: Sending complete: %d bytes", buf.size() );
	    emit completed();
	    mode = Idle;
	}
    }
}

void ServerDTP::readyRead()
{
    // retrieve file mode
    if ( RetrieveFile == mode ) {
	QCString s;
	s.resize( bytesAvailable() );
	readBlock( s.data(), bytesAvailable() );
	file.writeBlock( s.data(), s.size() );
    }

    // retrieve buffer mode
    else if ( RetrieveBuffer == mode ) {
	QCString s;
	s.resize( bytesAvailable() );
	readBlock( s.data(), bytesAvailable() );
	buf.writeBlock( s.data(), s.size() );
    }
}

void ServerDTP::sendFile( const QString fn, const QHostAddress& host, Q_UINT16 port )
{
    file.setName( fn );
    mode = SendFile;
    connectToHost( host.toString(), port );
}

void ServerDTP::sendFile( const QString fn )
{
    file.setName( fn );
    mode = SendFile;
}

void ServerDTP::retrieveFile( const QString fn, const QHostAddress& host, Q_UINT16 port )
{
    file.setName( fn );
    mode = RetrieveFile;
    connectToHost( host.toString(), port );
}

void ServerDTP::retrieveFile( const QString fn )
{
    file.setName( fn );
    mode = RetrieveFile;
}

void ServerDTP::sendByteArray( const QByteArray& array, const QHostAddress& host, Q_UINT16 port )
{
    buf.setBuffer( array );
    mode = SendBuffer;
    connectToHost( host.toString(), port );
}

void ServerDTP::sendByteArray( const QByteArray& array )
{
    buf.setBuffer( array );
    mode = SendBuffer;
}

void ServerDTP::retrieveByteArray( const QHostAddress& host, Q_UINT16 port )
{
    buf.setBuffer( QByteArray() );
    mode = RetrieveBuffer;
    connectToHost( host.toString(), port );
}

void ServerDTP::retrieveByteArray()
{
    buf.setBuffer( QByteArray() );
    mode = RetrieveBuffer;
}

void ServerDTP::setSocket( int socket )
{
    QSocket::setSocket( socket );
    connected();
}
