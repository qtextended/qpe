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

#include <qtimer.h>
#include <qcombobox.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qstyle.h>
#include <qpushbutton.h>
#include <qtabbar.h>
#include <qwidgetstack.h>
#include <qlayout.h>
#include <qregexp.h>
#include <qframe.h>
#include <qpeapplication.h>
#include <qpainter.h>
#include <qwindowsystem_qws.h>
#include "qcopenvelope_qws.h"
#include <resource.h>
#include <applnk.h>
#include <config.h>
#include "launcherview.h"
#include "launcher.h"
#include "global.h"
#include "lnkproperties.h"
#include "../taskbar/mrulist.h"
#include "mimetype.h"
#include <qdir.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

//#define SHOW_ALL

CategoryTabWidget::CategoryTabWidget( QWidget* parent ) :
    QVBox( parent )
{
    categoryBar = 0;
    stack = 0;
}

void CategoryTabWidget::prevTab()
{
    if ( categoryBar ) {
	int n = categoryBar->count();
	int tab = categoryBar->currentTab();
	if ( tab >= 0 )
            categoryBar->setCurrentTab( (tab - 1 + n)%n );
    }
}

void CategoryTabWidget::nextTab()
{
    if ( categoryBar ) {
	int n = categoryBar->count();
        int tab = categoryBar->currentTab();
	categoryBar->setCurrentTab( (tab + 1)%n );
    }
}

void CategoryTabWidget::addItem( const QString& linkfile )
{
    int i=0;
    AppLnk *app = new AppLnk(linkfile);
    if ( !app->isValid() ) {
	delete app;
	return;
    }
    if ( !app->file().isEmpty() ) {
	// A document
	delete app;
	app = new DocLnk(linkfile);
	((LauncherView*)(stack->widget(ids.count()-1)))->addItem(app);
	return;
    }
    for ( QStringList::Iterator it=ids.begin(); it!=ids.end(); ++it) {
	if ( !(*it).isEmpty() ) {
	    QRegExp tf(*it,FALSE,TRUE);
	    if ( tf.match(app->type()) >= 0 ) {
		((LauncherView*)stack->widget(i))->addItem(app);
		return;
	    }
	    i++;
	}
    }
}

void CategoryTabWidget::initializeCategories(AppLnkSet* rootFolder, AppLnkSet* docFolder)
{
    delete categoryBar;
    categoryBar = new CategoryTabBar( this );
    QPalette pal = categoryBar->palette();
    pal.setColor( QColorGroup::Light, pal.color(QPalette::Active,QColorGroup::Shadow) );
    pal.setColor( QColorGroup::Background, pal.active().background().light(110) );
    categoryBar->setPalette( pal );

    delete stack;
    stack = new QWidgetStack(this);
    tabs=0;

    ids.clear();

    QStringList types = rootFolder->types();
    for ( QStringList::Iterator it=types.begin(); it!=types.end(); ++it) {
	if ( !(*it).isEmpty() ) {
	    newView(*it,rootFolder->typePixmap(*it),rootFolder->typeName(*it));
	}
    }
    QListIterator<AppLnk> it( rootFolder->children() );
    AppLnk* l;
    while ( (l=it.current()) ) {
	if ( l->type() == "Separator" ) {
	    rootFolder->remove(l);
	    delete l;
	} else {
	    int i=0;
	    for ( QStringList::Iterator it=types.begin(); it!=types.end(); ++it) {
		if ( *it == l->type() )
		    ((LauncherView*)stack->widget(i))->addItem(l,FALSE);
		i++;
	    }
	}
	++it;
    }
    rootFolder->detachChildren();
    for (int i=0; i<tabs; i++)
	((LauncherView*)stack->widget(i))->sort();

    // all documents
    docview = newView( QString::null, Resource::loadPixmap("DocsIcon"), tr("Documents"));
    docview->setToolsEnabled(TRUE);
    docview->populate( docFolder, QString::null );
    docFolder->detachChildren();

    connect( categoryBar, SIGNAL(selected(int)), stack, SLOT(raiseWidget(int)) );

    ((LauncherView*)stack->widget(0))->setFocus();

    categoryBar->show();
    stack->show();
}

void CategoryTabWidget::updateDocs(AppLnkSet* docFolder)
{
    docview->populate( docFolder, QString::null );
    docFolder->detachChildren();
}

LauncherView* CategoryTabWidget::newView( const QString& id, const QPixmap& pm, const QString& label )
{
    LauncherView* view = new LauncherView( stack );
    connect( view, SIGNAL(clicked(const AppLnk*)),
	    this, SIGNAL(clicked(const AppLnk*)));
    connect( view, SIGNAL(rightPressed(AppLnk*)),
	    this, SIGNAL(rightPressed(AppLnk*)));
    ids.append(id);
    categoryBar->addTab( new QTab( pm, label ) );
    stack->addWidget( view, tabs++ );
    return view;
}

void CategoryTabWidget::updateLink(const QString& linkfile)
{
    int i=0;
    LauncherView* view;
    while ((view = (LauncherView*)stack->widget(i++))) {
	if ( view->removeLink(linkfile) )
	    break;
    }
    addItem(linkfile);
    docview->updateTools();
}

void CategoryTabWidget::paletteChange( const QPalette &p )
{
    QVBox::paletteChange( p );
    QPalette pal = palette();
    pal.setColor( QColorGroup::Light, pal.color(QPalette::Active,QColorGroup::Shadow) );
    pal.setColor( QColorGroup::Background, pal.active().background().light(110) );
    categoryBar->setPalette( pal );
    categoryBar->update();
}

void CategoryTabWidget::setBusy(bool on)
{
    if ( on )
	((LauncherView*)stack->visibleWidget())->setBusy(TRUE);
    else
	for (int i=0; i<tabs; i++)
	    ((LauncherView*)stack->widget(i))->setBusy(FALSE);
}


CategoryTabBar::CategoryTabBar( QWidget *parent, const char *name )
    : QTabBar( parent, name )
{
    setFocusPolicy( NoFocus );
    connect( this, SIGNAL( selected(int) ), this, SLOT( layoutTabs() ) );
}

CategoryTabBar::~CategoryTabBar()
{
}

void CategoryTabBar::layoutTabs()
{
    if ( !count() )
	return;

//    int percentFalloffTable[] = { 100, 70, 40, 12, 6, 3, 1, 0 };
    int hiddenTabWidth = -12;
    int middleTab = currentTab();
    int hframe, vframe, overlap;
    style().tabbarMetrics( this, hframe, vframe, overlap );
    QFontMetrics fm = fontMetrics();
    int x = 0;
    QRect r;
    QTab *t;
    int available = width()-1;
    int required = 0;
    for ( int i = 0; i < count(); i++ ) {
	t = tab(i);
	// if (( i < (middleTab - 1) ) || ( i > (middleTab + 1) )) {
	if ( i != middleTab ) {
	    // required += hiddenTabWidth + hframe - overlap;
	    available -= hiddenTabWidth + hframe - overlap;
	    if ( t->iconSet() != 0 )
		available -= t->iconSet()->pixmap( QIconSet::Small, QIconSet::Normal ).width();
	} else {
	    required += fm.width( t->text() ) + hframe - overlap;
	    if ( t->iconSet() != 0 )
		required += t->iconSet()->pixmap( QIconSet::Small, QIconSet::Normal ).width();
	}
    }
    for ( int i = 0; i < count(); i++ ) {
	t = tab(i);
	// if (( i < (middleTab - 1) ) || ( i > (middleTab + 1) )) {
	if ( i != middleTab ) {
	    int w = hiddenTabWidth;
	    int ih = 0;
	    if ( t->iconSet() != 0 ) {
		w += t->iconSet()->pixmap( QIconSet::Small, QIconSet::Normal ).width();
		ih = t->iconSet()->pixmap( QIconSet::Small, QIconSet::Normal ).height();
	    }
	    int h = QMAX( fm.height(), ih );
	    h = QMAX( h, QApplication::globalStrut().height() );

	    h += vframe;
	    w += hframe;

	    t->setRect( QRect(x, 0, w, h) );
	    x += t->rect().width() - overlap;
	    r = r.unite( t->rect() );
	} else {
	    int w = fm.width( t->text() );
	    int ih = 0;
	    if ( t->iconSet() != 0 ) {
		w += t->iconSet()->pixmap( QIconSet::Small, QIconSet::Normal ).width();
		ih = t->iconSet()->pixmap( QIconSet::Small, QIconSet::Normal ).height();
	    }
	    int h = QMAX( fm.height(), ih );
	    h = QMAX( h, QApplication::globalStrut().height() );

	    h += vframe;
	    w += hframe;

	    // t->setRect( QRect(x, 0, w * available/required, h) );
	    t->setRect( QRect(x, 0, available, h) );
	    x += t->rect().width() - overlap;
	    r = r.unite( t->rect() );
	}
    }

    QRect rr = tab(count()-1)->rect();
    rr.setRight(width()-1);
    tab(count()-1)->setRect( rr );

    for ( t = tabList()->first(); t; t = tabList()->next() ) {
       QRect tr = t->rect();
       tr.setHeight( r.height() );
       t->setRect( tr );
    }

    update();
}


void CategoryTabBar::paint( QPainter * p, QTab * t, bool selected ) const
{
#if QT_VERSION >= 300
    QStyle::SFlags flags = QStyle::Style_Default;
    if ( selected )
        flags |= QStyle::Style_Selected;
    style().drawControl( QStyle::CE_TabBarTab, p, this, t->rect(),
                         colorGroup(), flags, QStyleOption(t) );
#else
    style().drawTab( p, this, t, selected );
#endif

    QRect r( t->rect() );
    QFont f( font() );
    if ( selected )
	f.setBold( TRUE );
    p->setFont( f );

    int iw = 0;
    int ih = 0;
    if ( t->iconSet() != 0 ) {
	iw = t->iconSet()->pixmap( QIconSet::Small, QIconSet::Normal ).width() + 2;
	ih = t->iconSet()->pixmap( QIconSet::Small, QIconSet::Normal ).height();
    }
    int w = iw + p->fontMetrics().width( t->text() ) + 4;
    int h = QMAX(p->fontMetrics().height() + 4, ih );
    paintLabel( p, QRect( r.left() + (r.width()-w)/2 - 3,
			  r.top() + (r.height()-h)/2, w, h ), t,
#if QT_VERSION >= 300
			    t->identifier() == keyboardFocusTab()
#else
			    t->identitifer() == keyboardFocusTab()
#endif
		);
}


void CategoryTabBar::paintLabel( QPainter* p, const QRect&,
			  QTab* t, bool has_focus ) const
{
    QRect r = t->rect();
    //    if ( t->id != currentTab() )
    //r.moveBy( 1, 1 );
    //
    if ( t->iconSet() ) {
	// the tab has an iconset, draw it in the right mode
	QIconSet::Mode mode = (t->isEnabled() && isEnabled()) ? QIconSet::Normal : QIconSet::Disabled;
	if ( mode == QIconSet::Normal && has_focus )
	    mode = QIconSet::Active;
	QPixmap pixmap = t->iconSet()->pixmap( QIconSet::Small, mode );
	int pixw = pixmap.width();
	int pixh = pixmap.height();
	p->drawPixmap( r.left() + 6, r.center().y() - pixh / 2 + 1, pixmap );
	r.setLeft( r.left() + pixw + 5 );
    }

    QRect tr = r;

    if ( r.width() < 20 )
	return;

    if ( t->isEnabled() && isEnabled()  ) {
#if defined(_WS_WIN32_)
	if ( colorGroup().brush( QColorGroup::Button ) == colorGroup().brush( QColorGroup::Background ) )
	    p->setPen( colorGroup().buttonText() );
	else
	    p->setPen( colorGroup().foreground() );
#else
	p->setPen( colorGroup().foreground() );
#endif
	p->drawText( tr, AlignCenter | AlignVCenter | ShowPrefix, t->text() );
    } else {
	p->setPen( palette().disabled().foreground() );
	p->drawText( tr, AlignCenter | AlignVCenter | ShowPrefix, t->text() );
    }
}

//---------------------------------------------------------------------------

Launcher::Launcher( QWidget* parent, const char* name, WFlags fl )
    : QMainWindow( parent, name, fl )
{
    setCaption( tr("Launcher") );

    // we have a pretty good idea how big we'll be
    setGeometry( 0, 0, qApp->desktop()->width(), qApp->desktop()->height() );

    tabs = 0;
    rootFolder = 0;
    docsFolder = 0;

    tabs = new CategoryTabWidget( this );
    tabs->setMaximumWidth( qApp->desktop()->width() );
    setCentralWidget( tabs );

    connect( tabs, SIGNAL(selected(const QString&)),
	this, SLOT(viewSelected(const QString&)) );
    connect( tabs, SIGNAL(clicked(const AppLnk*)),
	this, SLOT(select(const AppLnk*)));
    connect( tabs, SIGNAL(rightPressed(AppLnk*)),
	this, SLOT(properties(AppLnk*)));
#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
    QCopChannel* sysChannel = new QCopChannel( "QPE/System", this );
    connect( sysChannel, SIGNAL(received(const QCString &, const QByteArray &)),
             this, SLOT(systemMessage( const QCString &, const QByteArray &)) );
#endif

    MimeType::updateApplications();
    updateTabs();

    preloadApps();

    startTimer(2000);
}

void Launcher::timerEvent(QTimerEvent*)
{
    struct stat s;
    ::stat("/var/run/stab", &s);
    if ( (long)s.st_mtime != lastStabTime ) {
	lastStabTime = (long)s.st_mtime;
	updateDocs();
    }
}

Launcher::~Launcher()
{
}

static bool isVisibleWindow(int wid)
{
    const QList<QWSWindow> &list = qwsServer->clientWindows();
    QWSWindow* w;
    for (QListIterator<QWSWindow> it(list); (w=it.current()); ++it) {
	if ( w->winId() == wid )
	    return !w->isFullyObscured();
    }
    return FALSE;
}

void Launcher::showMaximized()
{
    if ( isVisibleWindow( winId() ) )
	doMaximize();
    else
	QTimer::singleShot( 20, this, SLOT(doMaximize()) );
}

void Launcher::doMaximize()
{
    QMainWindow::showMaximized();
}

void Launcher::updateMimeTypes()
{
    MimeType::clear();
    updateMimeTypes(rootFolder);
}

void Launcher::updateMimeTypes(AppLnkSet* folder)
{
    for ( QListIterator<AppLnk> it( folder->children() ); it.current(); ++it ) {
	AppLnk *app = it.current();
	if ( app->type() == "Folder" )
	    updateMimeTypes((AppLnkSet *)app);
	else {
	    MimeType::registerApp(*app);
	}
    }
}


void Launcher::updateTabs()
{
    delete rootFolder;
    rootFolder = new AppLnkSet( MimeType::appsFolderName() );

    delete docsFolder;
    docsFolder = new DocLnkSet( docsFolderName() );

    DocLnkSet ide("/mnt/ide");
    docsFolder->appendFrom(ide);

    tabs->initializeCategories(rootFolder, docsFolder);
}

void Launcher::updateDocs()
{
    delete docsFolder;
    docsFolder = new DocLnkSet( docsFolderName() );
    DocLnkSet ide("/mnt/ide");
    docsFolder->appendFrom(ide);

    tabs->updateDocs(docsFolder);
}

QString Launcher::docsFolderName()
{
    return QString(getenv("HOME")) + "/Documents";
}

void Launcher::viewSelected(const QString& s)
{
    setCaption( s + tr(" - Launcher") );
}

void Launcher::nextView()
{
    tabs->nextTab();
}


void Launcher::select( const AppLnk *appLnk )
{
    if ( appLnk->type() == "Folder" ) {
	// Not supported: flat is simpler for the user
    } else {
	tabs->setBusy(TRUE);
	emit executing( appLnk );
	MRUList::addTask( appLnk );
	appLnk->execute();
    }
}

void Launcher::properties( AppLnk *appLnk )
{
    if ( appLnk->type() == "Folder" ) {
	// Not supported: flat is simpler for the user
    } else {
	LnkProperties prop(appLnk);
	prop.showMaximized();
	prop.exec();
    }
}

void Launcher::systemMessage( const QCString &msg, const QByteArray &data)
{
    if ( msg == "linkChanged(QString)" ) {
	QDataStream stream( data, IO_ReadOnly );
	QString link;
	stream >> link;
	if (link.isNull())
	    updateTabs();
	else if (link.isEmpty())
	    updateDocs();
	else
	    tabs->updateLink(link);
    } else if ( msg == "busy()" ) {
	emit busy();
    } else if ( msg == "notBusy()" ) {
	tabs->setBusy(FALSE);
	emit notBusy();
    } else if ( msg == "mkdir(QString)" ) {
	QDataStream stream( data, IO_ReadOnly );
	QString dir;
	stream >> dir;
	if ( !dir.isEmpty() )
	    mkdir( dir );
    }

}

bool Launcher::mkdir(const QString &localPath)
{
    QDir fullDir(localPath);
    if (fullDir.exists())
	return true;

    // at this point the directory doesn't exist
    // go through the directory tree and start creating the direcotories
    // that don't exist; if we can't create the directories, return false

    QString dirSeps = "/";
    int dirIndex = localPath.find(dirSeps);
    QString checkedPath;

    // didn't find any seps; weird, use the cur dir instead
    if (dirIndex == -1) {
	//qDebug("No seperators found in path %s", localPath.latin1());
	checkedPath = QDir::currentDirPath();
    }

    while (checkedPath != localPath) {
	// no more seperators found, use the local path
	if (dirIndex == -1)
	    checkedPath = localPath;
	else {
	    // the next directory to check
	    checkedPath = localPath.left(dirIndex) + "/";
	    // advance the iterator; the next dir seperator
	    dirIndex = localPath.find(dirSeps, dirIndex+1);
	}

	QDir checkDir(checkedPath);
	if (!checkDir.exists()) {
	    //qDebug("mkdir making dir %s", checkedPath.latin1());

	    if (!checkDir.mkdir(checkedPath)) {
		qDebug("Unable to make directory %s", checkedPath.latin1());
		return FALSE;
	    }
	}

    }
    return TRUE;
}

void Launcher::preloadApps()
{
    Config cfg("Launcher");
    cfg.setGroup("Preload");
    QStringList apps = cfg.readListEntry("Apps",',');
    for (QStringList::ConstIterator it=apps.begin(); it!=apps.end(); ++it) {
	Global::execute((*it) + " -preload");
    }
}
