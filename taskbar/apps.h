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

// To add a new app, include appropriate files and add another APP().

#ifndef APP_INCLUDES
#define APP_INCLUDES

#include "../addressbook/addressbook.h"
#include "../datebook/datebook.h"
#include "../helpbrowser/helpbrowser.h"
#include "../minesweep/minesweep.h"
#include "../textedit/textedit.h"
#include "../todo/mainwindow.h"
#include "../citytime/citytime.h"
#include "../clock/clock.h"
#include "../calculator/calculatorimpl.h"
#include "../tetrix/qtetrix.h"
#include "../sysinfo/sysinfo.h"
#include "../settings/appearance/settings.h"
#include "../settings/language/settings.h"
#include "../settings/light-and-power/settings.h"
#include "../settings/rotation/settings.h"
#include "../settings/settime.h"
#if !defined(QT_QPE_SMALL_BUILD)
#include "../canvas/canvas.h"
#include "../filebrowser/filebrowser.h"
#include "../solitaire/solitaire.h"
#include "../spreadsheet/pimspreadsheetwindow.h"
#include "../snake/interface.h"
#include "../tux/tux.h"
#include "../parashoot/interface.h"
#endif
#if !defined(QT_DEMO_SINGLE_FLOPPY) && !defined(QT_QPE_SMALL_BUILD)
#include "../embeddedkonsole/konsole.h"
#include "../wordgame/wordgame.h"
#include "../mpegplayer/mpegplayer.h"
#endif

#endif

//   app-id             class                    maximize?   documentary?

APP( "addressbook",	AddressbookWindow,	 1,		0 )
APP( "datebook",	DateBook,		 1,		0 )
APP( "helpbrowser",	HelpBrowser,		 1,		1 )
APP( "textedit",	TextEdit,		 1,		1 )
APP( "todo",		TodoWindow,		 1,		0 )
APP( "calculator",	CalculatorImpl,		 1,		0 )
APP( "citytime",	CityTime,		 1,		0 )
APP( "clock",		Clock,			 0,		0 )
APP( "minesweep",	MineSweep,		 1,		0 )
APP( "tetrix",		QTetrix,		 1,		0 )
APP( "sysinfo",		SystemInfo,		 0,		0 )
APP( "appearance",	AppearanceSettings,	 1,		0 )
APP( "systemtime",	SetDateTime,		 1,		0 )
#if !defined(QT_QPE_SMALL_BUILD)
APP( "language",	LanguageSettings,	 1,		0 )
APP( "light-and-power", LightSettings,		 1,		0 )
//APP( "sound",		SoundSettings,		 1,		0 )
APP( "canvas",		CanvasWindow,	    	 1,		0 )
APP( "filebrowser",	FileBrowser,		 1,		0 )
APP( "solitaire",	CanvasCardWindow,	 1,		0 )
APP( "spreadsheet",	PIMSpreadsheetWindow,	 1,		1 )
APP( "snake",		SnakeGame,		 1,		0 )
APP( "rotation",	RotationSettings,	 1,		0 )
APP( "tux",		Tux,			 0,		0 )
APP( "parashoot",	ParaShoot,		 1,		0 )
#endif
#if !defined(QT_DEMO_SINGLE_FLOPPY) && !defined(QT_QPE_SMALL_BUILD)
APP( "embeddedkonsole",	Konsole,		 1,		0 )
APP( "wordgame",	WordGame,		 1,		0 )
APP( "mpegplayer",	MpegPlayer,		 1,		1 )
#endif

