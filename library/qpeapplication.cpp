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
** $Id: qpeapplication.cpp,v 1.90 2001/09/11 02:10:12 warwick Exp $
**
**********************************************************************/
#include <stdlib.h>
#include <unistd.h>
#include <qfile.h>
#ifdef Q_WS_QWS
#ifndef QT_NO_COP
#if QT_VERSION <= 231
#define private public
#define sendLocally processEvent
#include "qcopenvelope_qws.h"
#undef private
#else
#include "qcopenvelope_qws.h"
#endif
#endif
#include <qwindowsystem_qws.h>
#endif
#include <qtextstream.h>
#include <qpalette.h>
#include <qbuffer.h>
#include <qptrdict.h>
#include <qregexp.h>
#include <qlabel.h>
#include <qdialog.h>
#include "qpeapplication.h"
#include "qpestyle.h"
#if QT_VERSION >= 300
#include <qstylefactory.h>
#else
#include <qplatinumstyle.h>
#include <qwindowsstyle.h>
#include <qmotifstyle.h>
#include <qmotifplusstyle.h>
#include "lightstyle.h"
#endif
#include "global.h"
#include "resource.h"
#if QT_VERSION <= 230 && defined(QT_NO_CODECS)
#include "qutfcodec.h"
#endif
#include "config.h"
#include "fontmanager.h"
#include "alarmserver.h"

// for setBacklight()
#if defined(QT_QWS_IPAQ) || defined(QT_QWS_EBX)
#include <unistd.h>
#include <linux/fb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#endif
#include <stdlib.h>
#if defined(QT_QWS_IPAQ)
#include <linux/h3600_ts.h>
#endif


class QPEApplicationData {
public:
    QPEApplicationData() : presstimer(0), presswidget(0), rightpressed(FALSE),
	kbgrabber(0), notbusysent(FALSE), preloaded(FALSE), nomaximize(FALSE),
	qpe_main_widget(0)
    {
	qcopq.setAutoDelete(TRUE);
    }

    int presstimer;
    QWidget* presswidget;
    QPoint presspos;
    bool rightpressed;
    int kbgrabber;
    bool notbusysent;
    QString appName;
    struct QCopRec {
	QCopRec(const QCString &ch, const QCString &msg,
                               const QByteArray &d) :
	    channel(ch), message(msg), data(d) { }

	QCString channel;
	QCString message;
	QByteArray data;
    };
    bool preloaded;
    bool nomaximize;
    QWidget* qpe_main_widget;

    QList<QCopRec> qcopq;

    void enqueueQCop(const QCString &ch, const QCString &msg,
                               const QByteArray &data)
    {
	qcopq.append(new QCopRec(ch,msg,data));
    }
    void sendQCopQ()
    {
	QCopRec* r;
	for (QListIterator<QCopRec> it(qcopq); (r=it.current()); ++it)
	    QCopChannel::sendLocally(r->channel,r->message,r->data);
	qcopq.clear();
    }
};

static int& hack(int& i)
{
#if QT_VERSION <= 230 && defined(QT_NO_CODECS)
    // These should be created, but aren't in Qt 2.3.0
    (void)new QUtf8Codec;
    (void)new QUtf16Codec;
#endif
    return i;
}

static void setScreenSaverInterval(int interval)
{
    if ( interval == -1 ) {
	// Restore screen blanking and power saving state
	Config config( "qpe" );
	config.setGroup( "Screensaver" );
	if ( config.readNumEntry("Dim",0) == 0 )
	    interval = 0;
	else
	    interval = config.readNumEntry( "Interval", 180000 ); // default to 3 minutes
    }
    QWSServer::setScreenSaverInterval(interval); 
}


/*!
  \class QPEApplication qpeapplication.h
  \brief The QPEApplication class implements various system services
    that are available to all Qt Palmtop applications.

  Simply by using QPEApplication instead of QApplication, a plain Qt
  application becomes a Qt Palmtop application. It automatically follows
  style changes, quits and raises, and in the
  case of \link docwidget.html document-oriented\endlink applications,
  changes the current displayed document in response to the environment.
*/

/*!
  \fn void QPEApplication::clientMoused()

  \internal
*/

/*!
  \fn void QPEApplication::timeChanged();

  This signal is emitted when the time jumps forward or backwards
  by more than the normal passage of time.
*/

/*!
  \fn void QPEApplication::clockChanged( bool ampm );

  This signal is emitted when the user changes the style
  of clock. If \a ampm is TRUE, the user wants a 12-hour
  AM/PM close, otherwise, they want a 24-hour clock.
*/

/*!
  \fn void QPEApplication::appMessage( const QCString& msg, const QByteArray& data )

  This signal is emitted when a message is received on the
  QPE/Application/<i>appname</i> QCop channel for this application.

  The slot to which you connect this signal uses \a msg and \a data
  in the following way:

\code
    void MyWidget::receive( const QCString& msg, const QByteArray& data )
    {
	QDataStream stream( data, IO_ReadOnly );
	if ( msg == "someMessage(int,int,int)" ) {
	    int a,b,c;
	    stream >> a >> b >> c;
	    ...
	} else if ( msg == "otherMessage(QString)" ) {
	    ...
	}
    }
\endcode

  \sa qcop.html
*/

/*!
  Constructs a QPEApplication just as you would construct
  a QApplication, passing \a argc, \a argv, and \a t.
*/
QPEApplication::QPEApplication( int& argc, char **argv, Type t )
    : QApplication( hack(argc), argv, t )
{
    d = new QPEApplicationData;

#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
    sysChannel = new QCopChannel( "QPE/System", this );
    connect( sysChannel, SIGNAL(received(const QCString &, const QByteArray &)),
	     this, SLOT(systemMessage( const QCString &, const QByteArray &)) );

    QCString channel = QCString(argv[0]);
    channel.replace(QRegExp(".*/"),"");
    d->appName = channel;
    channel = "QPE/Application/" + channel;
    pidChannel = new QCopChannel( channel, this);
    connect( pidChannel, SIGNAL(received(const QCString &, const QByteArray &)),
	    this, SLOT(pidMessage(const QCString &, const QByteArray &)));

    for (int a=0; a<argc; a++) {
	if ( qstrcmp(argv[a],"-qcop")==0 ) {
	    if ( a+1<argc ) {
		QFile f(argv[a+1]);
		if ( f.open(IO_ReadOnly) ) {
		    QDataStream ds(&f);
		    QCString channel, message;
		    QByteArray data;
		    ds >> channel >> message >> data;
		    d->enqueueQCop(channel,message,data);
		    f.close();
		    f.remove();
		}

		while (argv[a+2]) {
		    argv[a] = argv[a+2];
		    a++;
		}
		argc-=2;
	    }
	} else if ( qstrcmp(argv[a],"-preload")==0 ) {
	    argv[a] = argv[a+1];
	    a++;
	    d->preloaded = TRUE;
	}
    }
#endif

    qwsSetDecoration( new QPEDecoration() );

#ifndef QT_NO_TRANSLATION
    QString lang = getenv( "LANG" );

    /*
    Config config("qpe");
    config.setGroup( "Appearance" );
    lang = config.readEntry( "Language", lang );
    */

    QTranslator * trans = new QTranslator(this);
    QString tfn = qpeDir()+"/i18n/"+lang+"/"+d->appName+".qm";
    if ( trans->load( tfn ))
	installTranslator( trans );
    else
	delete trans;

    trans = new QTranslator(this);
    tfn = qpeDir()+"/i18n/"+lang+"/libqpe.qm";
    if ( trans->load( tfn ))
	installTranslator( trans );
    else
	delete trans;
    
    //###language/font hack; should look it up somewhere
    if ( lang == "ja" || lang == "zh_CN" || lang == "zh_TW" || lang == "ko" ) {
	QFont fn = FontManager::unicodeFont( FontManager::Proportional );
	setFont( fn );
    }
#endif

    applyStyle();

    if ( type() == GuiServer )
	setScreenSaverInterval(-1);
}

static QPtrDict<void>* inputMethodDict=0;
static void createInputMethodDict()
{
    if ( !inputMethodDict )
	inputMethodDict = new QPtrDict<void>;
}

/*!
  Returns the currently set hint to the system as to whether
  \a w has any use for text input methods.

  \sa setInputMethodHint()
*/
QPEApplication::InputMethodHint QPEApplication::inputMethodHint( QWidget* w )
{
    if ( inputMethodDict && w )
	return (InputMethodHint)(int)inputMethodDict->find(w);
    return Normal;
}

/*!
    \enum QPEApplication::InputMethodHint
 
    \value Normal the application sometimes needs text input (the default).
    \value AlwaysOff the application never needs text input.
    \value AlwaysOn the application always needs text input.
*/

/*!
  Hints to the system that \a w has use for text input methods
  as specified by \a mode.

  \sa inputMethodHint()
*/
void QPEApplication::setInputMethodHint( QWidget* w, InputMethodHint mode )
{
    createInputMethodDict();
    if ( mode == Normal ) {
	inputMethodDict->remove(w);
    } else {
	inputMethodDict->insert(w,(void*)mode);
    }
}

class HackDialog : public QDialog
{
public:
    void acceptIt() { accept(); }
};

/*!
  \internal
*/
bool QPEApplication::qwsEventFilter( QWSEvent *e )
{
    if ( !d->notbusysent && e->type == QWSEvent::Focus ) {
	if ( qApp->type() != QApplication::GuiServer )
	    QCopEnvelope ("QPE/System", "notBusy()" );
	d->notbusysent=TRUE;
    }
    if ( type() == GuiServer ) {
	switch ( e->type ) {
	    case QWSEvent::Mouse:
		if ( e->asMouse()->simpleData.state && !QWidget::find(e->window()) )
		    emit clientMoused();
	}
    }
    if ( e->type == QWSEvent::Key ) {
	if ( d->kbgrabber == 1 )
	    return TRUE;
	QWSKeyEvent *ke = (QWSKeyEvent *)e;
	if ( ke->simpleData.keycode == Qt::Key_F33 ) {
	    // Use special "OK" key to press "OK" on top level widgets
	    QWidget *active = activeWindow();
	    QWidget *popup = 0;
	    if ( active && active->isPopup() ) {
		popup = active;
		active = active->parentWidget();
	    }
	    if ( active && active->winId() == ke->simpleData.window &&
		 !active->testWFlags( WStyle_Customize|WType_Popup|WType_Desktop )) {
		if ( ke->simpleData.is_press ) {
		    if ( popup )
			popup->close();
		    if ( active->inherits( "QDialog" ) ) {
			HackDialog *d = (HackDialog *)active;
			d->acceptIt();
		    } else {
			active->close();
		    }
		}
		return TRUE;
	    }
	}
#if QT_VERSION < 231
    // Filter out the F4/Launcher key from apps
    // ### The launcher key may not always be F4 on all devices
	if ( ((QWSKeyEvent *)e)->simpleData.keycode == Qt::Key_F4 )
	    return TRUE;
#endif
    }
    if ( e->type == QWSEvent::Focus ) {
	QWSFocusEvent *fe = (QWSFocusEvent*)e;
	if ( !fe->simpleData.get_focus ) {
	    QWidget *active = activeWindow();
	    if ( active && active->isPopup() )
		active->close();
	}
	if ( fe->simpleData.get_focus && inputMethodDict ) {
	    InputMethodHint m = inputMethodHint( QWidget::find(e->window()) );
	    if ( m == AlwaysOff )
		Global::hideInputMethod();
	    if ( m == AlwaysOn )
		Global::showInputMethod();
	}
    }
    return QApplication::qwsEventFilter( e );
}

/*!
  Destroys the QPEApplication.
*/
QPEApplication::~QPEApplication()
{
    ungrabKeyboard();
#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
    // Need to delete QCopChannels early, since the display will
    // be gone by the time we get to ~QObject().
    delete sysChannel;
    delete pidChannel;
#endif
    delete d;
}

/*!
  Returns <tt>$QPEDIR/</tt>.
*/
QString QPEApplication::qpeDir()
{
    const char *base = getenv( "QPEDIR" );
    if ( base )
	return QString( base ) + "/";

    return QString( "../" );
}

/*!
  Returns the user's current Document directory. There is a trailing "/".
*/
QString QPEApplication::documentDir()
{
    const char *base = getenv( "HOME" );
    if ( base )
	return QString( base ) + "/Documents/";

    return QString( "../Documents/" );
}

static int deforient=-1;

/*!
  \internal
*/
int QPEApplication::defaultRotation()
{
    if ( deforient < 0 ) {
	QString d = getenv("QWS_DISPLAY");
	if ( d.contains("Rot90") ) {
	    deforient = 90;
	} else if ( d.contains("Rot180") ) {
	    deforient = 180;
	} else if ( d.contains("Rot270") ) {
	    deforient = 270;
	} else {
	    deforient=0;
	}
    }
    return deforient;
}

/*!
  \internal
*/
void QPEApplication::setDefaultRotation(int r)
{
    if ( qApp->type() == GuiServer ) {
	deforient = r;
	setenv("QWS_DISPLAY", QString("Transformed:Rot%1:0").arg(r).latin1(), 1);
    } else {
	QCopEnvelope("QPE/System", "setDefaultRotation(int)") << r;
    }
}

/*!
  \internal
*/
void QPEApplication::applyStyle()
{
    Config config( "qpe" );

    config.setGroup( "Appearance" );

    // Widget style
    QString style = config.readEntry( "Style", "Light" );
    internalSetStyle( style );

    // Colors
    QColor bgcolor( config.readEntry( "Background", "#E5E1D5" ) );
    QColor btncolor( config.readEntry( "Button", "#D6CDBB" ) );
    QPalette pal( btncolor, bgcolor );
    QString color = config.readEntry( "Highlight", "#800000" );
    pal.setColor( QColorGroup::Highlight, QColor(color) );
    color = config.readEntry( "HighlightedText", "#FFFFFF" );
    pal.setColor( QColorGroup::HighlightedText, QColor(color) );
    color = config.readEntry( "Text", "#000000" );
    pal.setColor( QColorGroup::Text, QColor(color) );
    color = config.readEntry( "ButtonText", "#000000" );
    pal.setColor( QPalette::Active, QColorGroup::ButtonText, QColor(color) );
    color = config.readEntry( "Base", "#FFFFFF" );
    pal.setColor( QColorGroup::Base, QColor(color) );

    pal.setColor( QPalette::Disabled, QColorGroup::Text,
		  pal.color(QPalette::Active, QColorGroup::Background).dark() );

    setPalette( pal, TRUE );
}

static void setBacklight(int bright)
{
    if ( bright == -1 ) {
	// Read from config
	Config config( "qpe" );
	config.setGroup( "Screensaver" );
	bright = config.readNumEntry("Brightness",128);
    }
#if defined(QT_QWS_IPAQ) || defined(QT_QWS_EBX)
    if ( QFile::exists("/usr/bin/bl") ) {
	QString cmd = "/usr/bin/bl 1 ";
	cmd += bright<=0 ? "0 " : "1 ";
	cmd += QString::number(bright);
	system(cmd.latin1());
#if defined(QT_QWS_EBX)
    } else if ( QFile::exists("/dev/fl") ) {
#define FL_IOCTL_STEP_CONTRAST    100
	int fd = open("/dev/fl", O_WRONLY);
	if (fd >= 0 ) {
	    int steps = 5;
	    int bl = ( bright * steps + 127 ) / 255;
	    bl = ioctl(fd, FL_IOCTL_STEP_CONTRAST, bl);
	    // bright = ( bl * 255 + steps/2 ) / steps;
	    close(fd);
	}
    }
#elif defined(QT_QWS_IPAQ)
    } else if ( QFile::exists("/dev/ts") ) {
	typedef struct {
	    unsigned char mode;
	    unsigned char pwr;
	    unsigned char brightness;
	} FLITE_IN;
	int fd = open("/dev/ts", O_WRONLY);
	if (fd >= 0 ) {
	    FLITE_IN bl;
	    bl.mode = 1;
	    bl.pwr = bright ? 1 : 0;
	    bl.brightness = (bright * 100)/255;
	    ioctl(fd, FLITE_ON, &bl);
	    close(fd);
	}
    }
#endif
#endif
}

void QPEApplication::systemMessage( const QCString &msg, const QByteArray &data)
{
#ifdef Q_WS_QWS
    QDataStream stream( data, IO_ReadOnly );
    if ( msg == "applyStyle()" ) {
	applyStyle();
    } else if ( msg == "setScreenSaverInterval(int)" ) {
	if ( type() == GuiServer ) {
	    int time;
	    stream >> time;
	    setScreenSaverInterval(time);
	}
    } else if ( msg == "setBacklight(int)" ) {
	if ( type() == GuiServer ) {
	    int bright;
	    stream >> bright;
	    setBacklight(bright);
	}
    } else if ( msg == "setDefaultRotation(int)" ) {
	if ( type() == GuiServer ) {
	    int r;
	    stream >> r;
	    setDefaultRotation(r);
	}
    } else if ( msg == "shutdown()" ) {
	if ( type() == GuiServer )
	    shutdown();
    } else if ( msg == "quit()" ) {
	if ( type() != GuiServer )
	    quit();
    } else if ( msg == "restart()" ) {
	if ( type() == GuiServer )
	    restart();
    } else if ( msg == "grabKeyboard(QString)" ) {
	QString who;
	stream >> who;
	if ( who.isEmpty() )
	    d->kbgrabber = 0;
	else if ( who != d->appName )
	    d->kbgrabber = 1;
    } else if ( msg == "language(QString)" ) {
	if ( type() == GuiServer ) {
	    QString l;
	    stream >> l;
	    QString cl = getenv("LANG");
	    if ( cl != l ) {
		if ( l.isNull() )
		    unsetenv( "LANG" );
		else
		    setenv( "LANG", l.latin1(), 1 );
		restart();
	    }
	}
    } else if ( msg == "timeChange(QString)" ) {
	QString t;
	stream >> t;
	if ( t.isNull() )
	    unsetenv( "TZ" );
	else
	    setenv( "TZ", t.latin1(), 1 );
	// emit the signal so everyone else knows...
	emit timeChanged();
    } else if ( msg == "execute(QString)" ) {
	if ( type() == GuiServer ) {
	    QString t;
	    stream >> t;
	    Global::execute( t );
	}
    } else if ( msg == "execute(QString,QString)" ) {
	if ( type() == GuiServer ) {
	    QString t,d;
	    stream >> t >> d;
	    Global::execute( t, d );
	}
    } else if ( msg == "addAlarm(QDateTime,QCString,QCString,int)" ) {
	if ( type() == GuiServer ) {
	    QDateTime when;
	    QCString channel, message;
	    int data;
	    stream >> when >> channel >> message >> data;
	    AlarmServer::addAlarm( when, channel, message, data );
	}
    } else if ( msg == "deleteAlarm(QDateTime,QCString,QCString,int)" ) {
	if ( type() == GuiServer ) {
	    QDateTime when;
	    QCString channel, message;
	    int data;
	    stream >> when >> channel >> message >> data;
	    AlarmServer::deleteAlarm( when, channel, message, data );
	}
    } else if ( msg == "clockChange(bool)" ) {
	int tmp;
	stream >> tmp;
	emit clockChanged( tmp );
    }
#endif
}

/*!
  \internal
*/
bool QPEApplication::raiseAppropriateWindow()
{
    bool r=FALSE;
    // ########## raise()ing main window should raise and set active
    // ########## it and then all childen. This belongs in Qt/Embedded
    QWidget *top = d->qpe_main_widget;
    if ( !top ) top =mainWidget();
    if ( top ) { 
	if ( top->isVisible() )
	    r = TRUE;
#ifdef Q_WS_QWS
	if ( !d->nomaximize )
	    top->showMaximized();
	else
#endif
	    top->show();
	top->raise();
	top->setActiveWindow();
    }
    QWidget *topm = activeModalWidget();
    if ( topm ) { 
	topm->show();
	topm->raise();
	topm->setActiveWindow();
	r = FALSE;
    }
    return r;
}

void QPEApplication::pidMessage( const QCString &msg, const QByteArray & data)
{
#ifdef Q_WS_QWS
    if ( msg == "quit()" ) {
	if ( d->preloaded && d->qpe_main_widget )
	    d->qpe_main_widget->hide();
	else
	    quit();
    } else if ( msg == "quitIfInvisible()" ) {
	if ( d->qpe_main_widget && !d->qpe_main_widget->isVisible() )
	    quit();
    } else if ( msg == "raise()" ) {
	raiseAppropriateWindow();
    } else {
	if ( msg == "setDocument(QString)" ) {
	    QDataStream stream( data, IO_ReadOnly );
	    QString doc;
	    stream >> doc;
	    QWidget *mw = mainWidget();
	    if ( mw )
		Global::setDocument( mw, doc );
	} else if ( msg == "nextView()" ) {
	    if ( raiseAppropriateWindow() )
		emit appMessage( msg, data);
	} else {
	    emit appMessage( msg, data);
	}
    }
#endif
}

/*!
  Sets \a mw as the mainWidget() and shows it. For small windows,
  consider passing TRUE for \a nomaximize rather than the default FALSE.

  \sa showMainDocumentWidget()
*/
void QPEApplication::showMainWidget( QWidget* mw, bool nomaximize )
{
    QApplication::setMainWidget(mw);
    d->sendQCopQ();
    d->nomaximize = nomaximize;
    d->qpe_main_widget = mw;
    if ( d->preloaded ) {
	QApplication::setMainWidget(0);
    } else {
#ifdef Q_WS_QWS
	if ( !nomaximize )
	    mw->showMaximized();
	else
#endif
	mw->show();
    }
}

/*!
  Sets \a mw as the mainWidget() and shows it. For small windows,
  consider passing TRUE for \a nomaximize rather than the default FALSE.

  This calls designates the application as
  a \link docwidget.html document-oriented\endlink application.

  The \a mw widget must have a slot: setDocument(const QString&).

  \sa showMainWidget()
*/
void QPEApplication::showMainDocumentWidget( QWidget* mw, bool nomaximize )
{
    QApplication::setMainWidget(mw);
    if ( mw && argc() == 2 )
	Global::setDocument( mw, QString::fromUtf8(argv()[1]) );
    d->nomaximize = nomaximize;
    d->sendQCopQ();
    d->qpe_main_widget = mw;
    if ( !d->preloaded ) {
#ifdef Q_WS_QWS
	if ( !nomaximize )
	    mw->showMaximized();
	else
#endif
	mw->show();
    }
}

/*!
  \internal
*/
void QPEApplication::internalSetStyle( const QString &style )
{
#if QT_VERSION >= 300
    if ( style == "QPE" ) {
	setStyle( new QPEStyle );
    } else {
	QStyle *s = QStyleFactory::create(style);
	if ( s ) setStyle(s);
    }
#else
    if ( style == "Windows" ) {
	setStyle( new QWindowsStyle );
    } else if ( style == "QPE" ) {
	setStyle( new QPEStyle );
    } else if ( style == "Light" ) {
	setStyle( new LightStyle );
    }
#ifndef QT_NO_STYLE_PLATINUM
    else if ( style == "Platinum" ) {
	setStyle( new QPlatinumStyle );
    }
#endif
#ifndef QT_NO_STYLE_MOTIF
    else if ( style == "Motif" ) {
	setStyle( new QMotifStyle );
    }
#endif
#ifndef QT_NO_STYLE_MOTIFPLUS
    else if ( style == "MotifPlus" ) {
	setStyle( new QMotifPlusStyle );
    }
#endif
#endif
}

/*!
  \internal
*/
void QPEApplication::prepareForTermination(bool willrestart)
{
    if ( willrestart ) {
	// Draw a big wait icon, the image can be altered in later revisions
//	QWidget *d = QApplication::desktop();
	QImage img = Resource::loadImage( "wait" );
	QPixmap pix;
	pix.convertFromImage(img.smoothScale(3*img.width(), 3*img.height()));
	QLabel *lblWait = new QLabel(0, "wait hack!", QWidget::WStyle_Customize | 
				  QWidget::WStyle_NoBorder | QWidget::WStyle_Tool );
	lblWait->setPixmap( pix );
	lblWait->setAlignment( QWidget::AlignCenter );
	lblWait->show();
	lblWait->showMaximized();
    }
#ifndef SINGLE_APP
    { QCopEnvelope envelope("QPE/System", "quit()"); }
    processEvents(); // ensure the message goes out.
    sleep(1); // You have 1 second to comply.
#endif
}

/*!
  \internal
*/
void QPEApplication::shutdown()
{
    // Implement in server's QPEApplication subclass
}

/*!
  \internal
*/
void QPEApplication::restart()
{
    // Implement in server's QPEApplication subclass
}

static QPtrDict<void>* stylusDict=0;
static void createDict()
{
    if ( !stylusDict )
	stylusDict = new QPtrDict<void>;
}

/*!
  Returns the current StylusMode for \a w.

  \sa setStylusOperation()
*/
QPEApplication::StylusMode QPEApplication::stylusOperation( QWidget* w )
{
    if ( stylusDict )
	return (StylusMode)(int)stylusDict->find(w);
    return LeftOnly;
}

/*!
    \enum QPEApplication::StylusMode
 
    \value LeftOnly the stylus only generates LeftButton
			events (the default).
    \value RightOnHold the stylus generates RightButton events
			if the user uses the press-and-hold gesture.
 
    See setStylusOperation().
*/

/*!
  Causes \a w to receive mouse events according to \a mode.

  \sa stylusOperation()
*/
void QPEApplication::setStylusOperation( QWidget* w, StylusMode mode )
{
    createDict();
    if ( mode == LeftOnly ) {
	stylusDict->remove(w);
	w->removeEventFilter(qApp);
    } else {
	stylusDict->insert(w,(void*)mode);
	connect(w,SIGNAL(destroyed()),qApp,SLOT(removeSenderFromStylusDict()));
	w->installEventFilter(qApp);
    }
}


/*!
  \reimp
*/
bool QPEApplication::eventFilter( QObject *o, QEvent *e )
{
    if ( e->type() >= QEvent::MouseButtonPress && e->type() <= QEvent::MouseMove ) {
	QMouseEvent* me = (QMouseEvent*)e;
	if ( me->button() == LeftButton ) {
	    StylusMode mode = (StylusMode)(int)stylusDict->find(o);
	    switch (mode) {
	      case RightOnHold:
		switch ( me->type() ) {
		  case QEvent::MouseButtonPress:
		    d->presstimer = startTimer(200); // #### pref.
		    d->presswidget = (QWidget*)o;
		    d->presspos = me->pos();
		    d->rightpressed = FALSE;
		    break;
		  case QEvent::MouseButtonRelease:
		    if ( d->presstimer ) {
			killTimer(d->presstimer);
			d->presstimer = 0;
		    }
		    if ( d->rightpressed && d->presswidget ) {
			// Right released
			postEvent( d->presswidget, 
			    new QMouseEvent( QEvent::MouseButtonRelease, me->pos(),
				    RightButton, LeftButton+RightButton ) );
			// Left released, off-widget
			postEvent( d->presswidget, 
			    new QMouseEvent( QEvent::MouseMove, QPoint(-1,-1),
				    LeftButton, LeftButton ) );
			postEvent( d->presswidget, 
			    new QMouseEvent( QEvent::MouseButtonRelease, QPoint(-1,-1),
				    LeftButton, LeftButton ) );
			d->rightpressed = FALSE;
			return TRUE; // don't send the real Left release
		    }
		    break;
		  default:
		    break;
		}
		break;
	      default:
		;
	    }
	}
    }
    return FALSE;
}

/*!
  \reimp
*/
void QPEApplication::timerEvent( QTimerEvent *e )
{
    if ( e->timerId() == d->presstimer && d->presswidget ) {
	// Right pressed
	postEvent( d->presswidget, 
	    new QMouseEvent( QEvent::MouseButtonPress, d->presspos,
			RightButton, LeftButton ) );
	killTimer( d->presstimer );
	d->presstimer = 0;
	d->rightpressed = TRUE;
    }
}

void QPEApplication::removeSenderFromStylusDict()
{
    stylusDict->remove((void*)sender());
    if ( d->presswidget == sender() )
	d->presswidget = 0;
}

/*!
  \internal
*/
bool QPEApplication::keyboardGrabbed() const
{
    return d->kbgrabber;
}


/*!
  Reverses the effect of grabKeyboard(). This is called automatically
  on program exit.
*/
void QPEApplication::ungrabKeyboard()
{
    QPEApplicationData* d = ((QPEApplication*)qApp)->d;
    if ( d->kbgrabber == 2 ) {
	QCopEnvelope e("QPE/System", "grabKeyboard(QString)" );
	e << QString::null;
    }
}

/*!
  Grabs the keyboard such that the system's application launching
  keys no longer work, and instead they are receivable by this
  application.

  \sa ungrabKeyboard()
*/
void QPEApplication::grabKeyboard()
{
    QPEApplicationData* d = ((QPEApplication*)qApp)->d;
    if ( qApp->type() == QApplication::GuiServer )
	d->kbgrabber = 0;
    else {
	QCopEnvelope e("QPE/System", "grabKeyboard(QString)" );
	e << d->appName;
	d->kbgrabber = 2; // me
    }
}

/*!
  \reimp
*/
int QPEApplication::exec()
{
    d->sendQCopQ();
    return QApplication::exec();
}

#if defined(QT_QWS_IPAQ) || defined(QT_QWS_EBX)

// The libraries with the skiff package (and possibly others) have
// completely useless implementations of builtin new and delete that
// use about 50% of your CPU. Here we revert to the simple libc
// functions.

void* operator new[](size_t size)
{
    return malloc(size);
}

void* operator new(size_t size)
{
    return malloc(size);
}

void operator delete[](void* p)
{
    free(p);
}

void operator delete[](void* p, size_t size)
{
    free(p);
}

void operator delete(void* p)
{
    free(p);
}

void operator delete(void* p, size_t size)
{
    free(p);
}

#endif

#if ( QT_VERSION <= 230 ) && !defined(SINGLE_APP)
#include <qwidgetlist.h>
#include <qgfx_qws.h>
extern QRect qt_maxWindowRect;
void qt_setMaxWindowRect(const QRect& r)
{
    qt_maxWindowRect = qt_screen->mapFromDevice(r,
	qt_screen->mapToDevice(QSize(qt_screen->width(),qt_screen->height())));
    // Re-resize any maximized windows
    QWidgetList* l = QApplication::topLevelWidgets();
    if ( l ) {
        QWidget *w = l->first();
        while ( w ) {
            if ( w->isVisible() && w->isMaximized() )
            {
                w->showMaximized();
            }
            w = l->next();
        }
        delete l;
    }
}
#endif
