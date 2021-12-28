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
#include <qpedebug.h>
#include "global.h"
#include "qdawg.h"
#include <qfile.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qmap.h>
#include <qdict.h>
#include <qdir.h>
#include <qmessagebox.h>
#include <qregexp.h>
#include "qpeapplication.h"
#include "resource.h"
#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
#include "qcopenvelope_qws.h"
#endif

#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <qwindowsystem_qws.h> // for qwsServer
#include <qdatetime.h>

#include "applnk.h"

class Emitter : public QObject {
    Q_OBJECT
public:
    Emitter( QWidget* receiver, const QString& document )
    {
	connect(this, SIGNAL(setDocument(const QString&)),
	    receiver, SLOT(setDocument(const QString&)));
	emit setDocument(document);
	disconnect(this, SIGNAL(setDocument(const QString&)),
	       receiver, SLOT(setDocument(const QString&)));
    }

signals:
    void setDocument(const QString&);
};


class StartingAppList : public QObject {
    Q_OBJECT
public:
    static void add( const QString& name );
    static bool isStarting( const QString name );
private slots:
    void handleNewChannel( const QString &);
private:
    StartingAppList( QObject *parent=0, const char* name=0 ) ;
    
    QDict<QTime> dict;
    static StartingAppList *appl;
};

StartingAppList* StartingAppList::appl = 0;

StartingAppList::StartingAppList( QObject *parent, const char* name )
    :QObject( parent, name ) 
{
#if QT_VERSION >= 232 && !defined(QT_NO_COP)
    connect( qwsServer, SIGNAL( newChannel(const QString&)),
	     this, SLOT( handleNewChannel(const QString&)) );
    dict.setAutoDelete( TRUE );
#endif
}

void StartingAppList::add( const QString& name )
{
#if QT_VERSION >= 232  && !defined(QT_NO_COP)
    if ( !appl )
	appl = new StartingAppList;
    QTime *t = new QTime;
    t->start();
    appl->dict.insert( "QPE/Application/" + name, t );
#endif
}

bool StartingAppList::isStarting( const QString name )
{
#if QT_VERSION >= 232  && !defined(QT_NO_COP)
    if ( appl ) {
	QTime *t  = appl->dict.find( "QPE/Application/" + name );
	if ( !t )
	    return FALSE;
	if ( t->elapsed() > 10000 ) {
	    // timeout in case of crash or something
	    appl->dict.remove( "QPE/Application/" + name );
	    return FALSE;
	}
	return TRUE;
    }
#endif
    return FALSE;
}

void StartingAppList::handleNewChannel( const QString & name )
{
#if QT_VERSION >= 232  && !defined(QT_NO_COP)
    dict.remove( name );
#endif
}

static bool docDirCreated = FALSE;
static QDawg* fixed_dawg = 0;
static QDict<QDawg> *named_dawg = 0;

static QString qpeDir()
{
    QString dir = getenv("QPEDIR");
    if ( dir.isEmpty() ) dir = "..";
    return dir;
}

static QString dictDir()
{
    return qpeDir() + "/etc/dict";
}

/*!
  \class Global global.h
  \brief The Global class collects application-wide global functions.
*/

/*!
  \internal
*/
Global::Global()
{
}

/*!
  Returns the unchangeable QDawg that contains general
  words for the current locale.

  \sa addedDawg()
*/
const QDawg& Global::fixedDawg()
{
    if ( !fixed_dawg ) {
	if ( !docDirCreated )
	    createDocDir();

	fixed_dawg = new QDawg;
	QString dawgfilename = dictDir() + "/dawg";
	QString lang = getenv( "LANG" );
	QString dawgfilename_lang = dawgfilename + "." + lang;
	QString words_lang = dictDir() + "/words." + lang;
	if ( QFile::exists(dawgfilename_lang) ||
	     QFile::exists(words_lang) )
	    dawgfilename = dawgfilename_lang;
	QFile dawgfile(dawgfilename);

	if ( !dawgfile.exists() ) {
	    QString fn = dictDir() + "/words";
	    if ( QFile::exists(words_lang) )
		fn = words_lang;
	    QFile in(fn);
	    if ( in.open(IO_ReadOnly) ) {
		fixed_dawg->createFromWords(&in);
		dawgfile.open(IO_WriteOnly);
		fixed_dawg->write(&dawgfile);
		dawgfile.close();
	    }
	} else {
	    fixed_dawg->readFile(dawgfilename);
	}
    }

    return *fixed_dawg;
}

/*!
  Returns the changeable QDawg that contains general
  words for the current locale.

  \sa fixedDawg()
*/
const QDawg& Global::addedDawg()
{
    return dawg("local");
}

/*!
  Returns the QDawg with the given \a name.
  This is an application-specific word list.

  \a name should not contain "/".
*/
const QDawg& Global::dawg(const QString& name)
{
    createDocDir();
    if ( !named_dawg )
	named_dawg = new QDict<QDawg>;
    QDawg* r = named_dawg->find(name);
    if ( !r ) {
	r = new QDawg;
	named_dawg->insert(name,r);
	QString dawgfilename = dictDir() + "/" + name + ".dawg";
	QFile dawgfile(dawgfilename);
	if ( dawgfile.open(IO_ReadOnly) )
	    r->readFile(dawgfilename);
    }
    return *r;
}

/*!
  Adds \a wordlist to the addedDawg().
*/
void Global::addWords(const QStringList& wordlist)
{
    addWords("local",wordlist);
}

/*!
  Adds \a wordlist to the dawg() named \a dictname.
*/
void Global::addWords(const QString& dictname, const QStringList& wordlist)
{
    QDawg& d = (QDawg&)dawg(dictname);
    QStringList all = d.allWords() + wordlist;
    d.createFromWords(all);

    QString dawgfilename = dictDir() + "/" + dictname + ".dawg";
    QFile dawgfile(dawgfilename);
    if ( dawgfile.open(IO_WriteOnly) ) {
	d.write(&dawgfile);
	dawgfile.close();
    }

    // #### Re-read the dawg here if we use mmap().

    // #### Signal other processes to re-read.
}


/*!
  Returns a full path for the application named \a appname, with the
  given \a filename.  If \a filename contains "/", it is the caller's
  responsibility to ensure the directories exist.
*/
QString Global::applicationFileName(const QString& appname, const QString& filename)
{
    QDir d;
    QString r = getenv("HOME");
    r += "/Applications/";
    d.mkdir(r);
    r += appname;
    d.mkdir(r);
    r += "/"; r += filename;
    return r;
}

/*!
  \internal
*/
void Global::createDocDir()
{
    if ( !docDirCreated ) {
	docDirCreated = TRUE;
	mkdir( QPEApplication::documentDir().latin1(), 0755 );
    }
}


/*!
  Displays a status \a message to the user. This generally appears
  in the taskbar for some amount of time, then disappears.
*/
void Global::statusMessage(const QString& message)
{
#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
    QCopEnvelope e( "QPE/TaskBar", "message(QString)" );
    e << message;
#endif
}

/*!
  \internal
*/
void Global::applyStyle()
{
#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
    QCopChannel::send( "QPE/System", "applyStyle()" );
#else
    ((QPEApplication *)qApp)->applyStyle(); // apply without needing QCop for floppy version
#endif
}

/*!
  \internal
*/
QWidget *Global::shutdown( bool )
{
#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
    QCopChannel::send( "QPE/System", "shutdown()" );
#endif
    return 0;
}

/*!
  \internal
*/
QWidget *Global::restart( bool )
{
#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
    QCopChannel::send( "QPE/System", "restart()" );
#endif
    return 0;
}

/*!
  Explicitly show the current input method.
*/
void Global::showInputMethod()
{
#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
    QCopChannel::send( "QPE/TaskBar", "showInputMethod()" );
#endif
}

/*!
  Explicitly hide the current input method.
*/
void Global::hideInputMethod()
{
#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
    QCopChannel::send( "QPE/TaskBar", "hideInputMethod()" );
#endif
}


#ifdef QT_NO_QWS_MULTIPROCESS
/*!
  \internal
*/
bool Global::commandExists( const QString &name )
{
    for (int i = 0; builtin[i].file; i++) {
	if ( builtin[i].file == name ) {
	    return TRUE;
	}
    }
    return FALSE;
}
#endif // QT_NO_QWS_MULTIPROCESS

Global::Command* Global::builtin=0;
QGuardedPtr<QWidget> *Global::running=0;

/*!
  \class Global::Command
  \brief The Global::Command class is internal.
  \internal
*/

/*!
  \internal
*/
void Global::setBuiltinCommands( Command* list )
{
    if ( running )
	delete [] running;

    builtin = list;
    int count = 0;
    while ( builtin[count].file )
	count++;

    running = new QGuardedPtr<QWidget> [ count ];
}

/*!
  \internal
*/
void Global::setDocument( QWidget* receiver, const QString& document )
{
    Emitter emitter(receiver,document);
}

/*!
  \internal
*/
void Global::terminate( const AppLnk* app )
{
    QCString channel = "QPE/Application/" + app->exec().utf8();
    if ( QCopChannel::isRegistered(channel) ) {
	QCopEnvelope e(channel, "quit()");
    }
}

/*!
  Executes application identfied by \a c, passing \a document.

  Note that you might be better off sending a QCop message to
  the application's QPE/Application/<i>appname</i> channel.
*/
void Global::execute( const QString &c, const QString& document )
{
    if ( qApp->type() != QApplication::GuiServer ) {
	// ask the server to do the work
#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
	if ( document.isNull() ) {
	    QCopEnvelope e( "QPE/System", "execute(QString)" );
	    e << c;
	} else {
	    QCopEnvelope e( "QPE/System", "execute(QString,QString)" );
	    e << c << document;
	}
#endif
	return;
    }

    // Attempt to execute the app using a builtin class for the app first
    // else try and find it in the bin directory
    for (int i = 0; builtin[i].file; i++) {
	if ( builtin[i].file == c ) {
	    if ( running[i] ) {
		if ( !document.isNull() && builtin[i].documentary )
		    setDocument(running[i], document);
		running[i]->raise();
		running[i]->show();
		running[i]->setActiveWindow();
	    } else {
		running[i] = builtin[i].func( builtin[i].maximized );
	    }
	    QCopEnvelope ("QPE/System", "notBusy()" ); // that was quick ;-)
	    return;
	}
    }

    // Convert the command line in to a list of arguments
    QStringList list = QStringList::split(QRegExp("  *"),c);

#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
    // see if the application is already running
    if ( QCopChannel::isRegistered( ("QPE/Application/" + list[0]).latin1() ) ) {
	if ( !document.isEmpty() ) {
	    QCopEnvelope env( ("QPE/Application/" + list[0]).latin1(), "setDocument(QString)" );
	    env << document;
	}
	QCopChannel::send( ("QPE/Application/" + list[0]).latin1(), "raise()" );
	return;
    }
    //see if it is being started
    if ( StartingAppList::isStarting( list[0] ) ) {
	return;
    }

#endif

#ifdef QT_NO_QWS_MULTIPROCESS
    QMessageBox::warning( 0, "Error", "Could not find the application " + c, "Ok", 0, 0, 0, 1 );
#else    

    if ( !document.isEmpty() )
	list.append(document);

    QStrList slist;
    unsigned int j;
    for ( j = 0; j < list.count(); j++ )
	slist.append( list[j].utf8() );
    
    const char **args = new (const char *)[slist.count() + 1];
    for ( j = 0; j < slist.count(); j++ )
	args[j] = slist.at(j);
    args[j] = NULL;

#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
    // an attempt to show a wait...
    // more logic should be used, but this will be fine for the moment...
    QCopEnvelope ( "QPE/System", "busy()" );
#endif

    if ( !::vfork() ) {
	for ( int fd = 3; fd < 100; fd++ )
	    ::close( fd );
	::setpgid( ::getpid(), ::getppid() );
	// Try bindir first, so that foo/bar works too
	::execv( qpeDir()+"/bin/"+args[0], (char * const *)args );
	::execvp( args[0], (char * const *)args );
	exit( -1 );
    }
    StartingAppList::add( list[0] );
#endif //QT_NO_QWS_MULTIPROCESS    
}


#include "global.moc"
