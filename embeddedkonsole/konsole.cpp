/* ---------------------------------------------------------------------- */
/*                                                                        */
/* [main.C]                        Konsole                                */
/*                                                                        */
/* ---------------------------------------------------------------------- */
/*                                                                        */
/* Copyright (c) 1997,1998 by Lars Doelle <lars.doelle@on-line.de>        */
/*                                                                        */
/* This file is part of Konsole, an X terminal.                           */
/*                                                                        */
/* The material contained in here more or less directly orginates from    */
/* kvt, which is copyright (c) 1996 by Matthias Ettrich <ettrich@kde.org> */
/*                                                                        */
/* ---------------------------------------------------------------------- */
/*									      */
/* Ported Konsole to Qt/Embedded                                              */
/*									      */
/* Copyright (C) 2000 by John Ryland <jryland@trolltech.com>                  */
/*									      */
/* -------------------------------------------------------------------------- */

#include <qdir.h>
#include <qevent.h>
#include <qdragobject.h>
#include <qobjectlist.h>
#include <qtoolbutton.h>
#include <qtoolbar.h>
#include <qpushbutton.h>
#include <qfontdialog.h>
#include <qglobal.h>
#include <qpainter.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qaction.h>
#include <qapplication.h>
#include <qfontmetrics.h>
#include "resource.h"
#include <qcombobox.h>
#include <qevent.h>

#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "konsole.h"
#include "keytrans.h"


// This could be configurable or dynamicly generated from the bash history
// file of the user
static const char *commonCmds[] =
{
    "ls ",
    //"ls -la ",
    "cd ",
    "pwd",
    //"cat",
    //"less ",
    //"vi ",
    //"man ",
    "echo ",
    "set ",
    //"ps",
    "ps aux\r",
    //"tar",
    //"tar -zxf",
    "grep ",
    //"grep -i",
    //"mkdir",
    "cp ",
    "mv ",
    "rm",
    "rmdir",
    //"chmod",
    //"su\r",
    "top\r",
    //"find",
    //"make",
    //"tail",
    "cardctl eject\r",
    "ifconfig ",
    "iwconfig eth0 ",
    "nc localhost 7777\r",
    "nc localhost 7776\r",
    //"mount /dev/hda1",

/*
    "gzip",
    "gunzip",
    "chgrp",
    "chown",
    "date",
    "dd",
    "df",
    "dmesg",
    "fuser",
    "hostname",
    "kill",
    "killall",
    "ln",
    "ping",
    "mount",
    "more",
    "sort",
    "touch",
    "umount",
    "mknod",
    "netstat",
*/

    "exit\r",
    NULL
};


Konsole::Konsole(QWidget* parent, const char* name, WFlags fl) :
    QMainWindow(parent, name, fl)
{
    QStrList args;
    init("/bin/sh",args);
}

Konsole::Konsole(const char* name, const char* _pgm, QStrList & _args, int)
 : QMainWindow(0, name)
{
    init(_pgm,_args);
}

void Konsole::closeEvent( QCloseEvent * )
{
    doneSession( se, 0 );
}

void Konsole::init(const char* _pgm, QStrList & _args)
{
  se = 0L;
  b_scroll = TRUE; // histon;
  n_keytab = 0;
  n_render = 0;

  setCaption( tr("embeddedKonsole") );
  setIcon( Resource::loadPixmap( "konsole" ) );

  QFont font( "Helvetica", 6, QFont::Normal );
  font.setFixedPitch( FALSE );
  QFont fixedFont( "Micro", 4, QFont::Normal );
  fixedFont.setFixedPitch( TRUE );

  //setFont( font ); // ME: the menu font size should be standard

  // create terminal emulation framework ////////////////////////////////////
  te = new TEWidget(this);
  te->setMinimumSize(150,70);    // allow resizing, cause resize in TEWidget
//  te->setBackgroundPixmap( Resource::loadPixmap("faded_bg") );
  te->setBackgroundColor( Qt::white );
  te->setVTFont(fixedFont);

  // create terminal toolbar ////////////////////////////////////////////////
  setToolBarsMovable( FALSE );
  QToolBar *menuToolBar = new QToolBar( this );
  menuToolBar->setHorizontalStretchable( TRUE );
  
  QToolBar *secondToolBar = new QToolBar( this );
  secondToolBar->setHorizontalStretchable( TRUE );

  QComboBox *commonCombo = new QComboBox( secondToolBar );
//  commonCombo->setEditable( TRUE );
  for (int i = 0; commonCmds[i] != NULL; i++)
    commonCombo->insertItem( commonCmds[i], i );
  connect( commonCombo, SIGNAL( activated(int) ), this, SLOT( enterCommand(int) ));

  QMenuBar *menuBar = new QMenuBar( menuToolBar );

  fontList = new QPopupMenu( this );
  fontList->insertItem( tr("Micro"), 0 );
  fontList->insertItem( tr("Small Fixed"), 1 );
  fontList->insertItem( tr("Medium Fixed"), 2 );
//   fontList->insertItem( tr("Small Helvetica"), 3 );
//   fontList->insertItem( tr("Large Helvetica"), 4 );
  connect( fontList, SIGNAL( activated(int) ), this, SLOT( fontChanged(int) ));
  fontChanged( 1 );

  menuBar->insertItem( tr("Font"), fontList );

  QToolBar *toolbar = menuToolBar; // new QToolBar( this );

  QAction *a;

  // Button Commands
  a = new QAction( tr("Enter"), Resource::loadPixmap( "konsole/enter" ), QString::null, 0, this, 0 );
  connect( a, SIGNAL( activated() ), this, SLOT( hitEnter() ) ); a->addTo( toolbar );
  a = new QAction( tr("Space"), Resource::loadPixmap( "konsole/space" ), QString::null, 0, this, 0 );
  connect( a, SIGNAL( activated() ), this, SLOT( hitSpace() ) ); a->addTo( toolbar );
  a = new QAction( tr("Tab"), Resource::loadPixmap( "konsole/tab" ), QString::null, 0, this, 0 );
  connect( a, SIGNAL( activated() ), this, SLOT( hitTab() ) ); a->addTo( toolbar );
  a = new QAction( tr("Up"), Resource::loadPixmap( "konsole/up" ), QString::null, 0, this, 0 );
  connect( a, SIGNAL( activated() ), this, SLOT( hitUp() ) ); a->addTo( toolbar );
  a = new QAction( tr("Down"), Resource::loadPixmap( "konsole/down" ), QString::null, 0, this, 0 );
  connect( a, SIGNAL( activated() ), this, SLOT( hitDown() ) ); a->addTo( toolbar );
  a = new QAction( tr("Paste"), Resource::loadPixmap( "paste" ), QString::null, 0, this, 0 );
  connect( a, SIGNAL( activated() ), this, SLOT( hitPaste() ) ); a->addTo( toolbar );
/*
  a = new QAction( tr("Up"), Resource::loadPixmap( "up" ), QString::null, 0, this, 0 );
  connect( a, SIGNAL( activated() ), this, SLOT( hitUp() ) ); a->addTo( toolbar );
  a = new QAction( tr("Down"), Resource::loadPixmap( "down" ), QString::null, 0, this, 0 );
  connect( a, SIGNAL( activated() ), this, SLOT( hitDown() ) ); a->addTo( toolbar );
*/

  // create applications /////////////////////////////////////////////////////
  setCentralWidget(te);

  // load keymaps ////////////////////////////////////////////////////////////
  KeyTrans::loadAll();
  for (int i = 0; i < KeyTrans::count(); i++)
  { KeyTrans* s = KeyTrans::find(i);
    assert( s );
  }

  // construct initial session ///////////////////////////////////////////////
  se = new TESession(this,te,_pgm,_args,"xterm");
  connect( se, SIGNAL(done(TESession*,int)), this, SLOT(doneSession(TESession*,int)) );
  se->setHistory(b_scroll);

  // read and apply default values ///////////////////////////////////////////
  resize(321, 321); // Dummy.
  QSize currentSize = size();
  if (currentSize != size())
     defaultSize = size();

  // activate and run first session //////////////////////////////////////////
  runSession(se);
}

Konsole::~Konsole()
{
    if (te)
	delete te;
}

void Konsole::fontChanged(int f)
{
    const char *name = 0;
    int size = 0;
    switch (f)
    {
        case 0:
            name = "Micro";
	    size = 4;
            break;
        case 1:
            name = "Fixed";
	    size = 7;
            break;
        case 2:
            name = "Fixed";
	    size =12;
            break;
//         case 3:
//             name = "Helvetica";
// 	    size = 6;
//             break;
//         case 4:
//             name = "Helvetica";
// 	    size =10;
//             break;
    }
    QFont fixedFont = QFont( name, size, QFont::Normal );

    for ( int i = 0; i < 3; i++ )
	fontList->setItemChecked( i, (i == f) ? TRUE : FALSE );

    fixedFont.setFixedPitch( (f < 3) ? TRUE : FALSE );
    te->setVTFont(fixedFont);
}

void Konsole::enterCommand(int c)
{
    QString text = commonCmds[c];
    te->emitText(text);
}

void Konsole::hitEnter()
{
    te->emitText(QString("\r"));
}

void Konsole::hitSpace()
{
    te->emitText(QString(" "));
}

void Konsole::hitTab()
{
    te->emitText(QString("\t"));
}

void Konsole::hitPaste()
{
    te->pasteClipboard();
}

void Konsole::hitUp()
{
    QKeyEvent ke( QKeyEvent::KeyPress, Qt::Key_Up, 0, 0);
    QApplication::sendEvent( te, &ke );
}

void Konsole::hitDown()
{
    QKeyEvent ke( QKeyEvent::KeyPress, Qt::Key_Down, 0, 0);
    QApplication::sendEvent( te, &ke );
}

/**
   This function calculates the size of the external widget
   needed for the internal widget to be
 */
QSize Konsole::calcSize(int columns, int lines) {
    QSize size = te->calcSize(columns, lines);
    return size;
}

/**
    sets application window to a size based on columns X lines of the te
    guest widget. Call with (0,0) for setting default size.
*/

void Konsole::setColLin(int columns, int lines)
{
  if ((columns==0) || (lines==0))
  {
    if (defaultSize.isEmpty()) // not in config file : set default value
    {
      defaultSize = calcSize(80,24);
      // notifySize(24,80); // set menu items (strange arg order !)
    }
    resize(defaultSize);
  } else {
    resize(calcSize(columns, lines));
    // notifySize(lines,columns); // set menu items (strange arg order !)
  }
}

/*
void Konsole::setFont(int fontno)
{
  QFont f;
  if (fontno == 0)
    f = defaultFont = QFont( "Helvetica", 12  );
  else
  if (fonts[fontno][0] == '-')
    f.setRawName( fonts[fontno] );
  else
  {
    f.setFamily(fonts[fontno]);
    f.setRawMode( TRUE );
  }
  if ( !f.exactMatch() && fontno != 0)
  {
    QString msg = i18n("Font `%1' not found.\nCheck README.linux.console for help.").arg(fonts[fontno]);
    QMessageBox(this,  msg);
    return;
  }
  if (se) se->setFontNo(fontno);
  te->setVTFont(f);
  n_font = fontno;
}
*/

// --| color selection |-------------------------------------------------------

void Konsole::changeColumns(int columns)
{
  setColLin(columns,te->Lines());
  te->update();
}

void Konsole::runSession(TESession* s)
{
    activateSession(s);
  // give some time to get through the
  // resize events before starting up.
    QTimer::singleShot(100,s,SLOT(run()));
}

void Konsole::activateSession(TESession *s)
{
  se = s;
  te->currentSession = se;
  s->setConnect(TRUE);
}


//FIXME: If a child dies during session swap,
//       this routine might be called before
//       session swap is completed.

void Konsole::doneSession(TESession* s, int )
{
  s->setConnect(FALSE);
  delete s;
  delete this;
}


