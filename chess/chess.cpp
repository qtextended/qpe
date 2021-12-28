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
** $Id: chess.cpp,v 1.6 2001/09/11 06:25:04 lukegraham Exp $
**
**********************************************************************/

// promotions dont replace the picture of the pawn with the
// new piece
// human is not offered a promotion
// en passant does not work (the captured pawn is not removed from the
// board)

#include <qcanvas.h>
#include <qmainwindow.h>
#include <qlist.h>
#include <qimage.h>
#include <qpainter.h>
#include <qmessagebox.h>
#include <qcstring.h>
#include <qregexp.h>

#include <config.h>
#include <resource.h>

#include "chess.h"

#define CHESS_DEBUG

QVector < QImage > Piece::imgList;
int
  Piece::pieceSize =
  40;

// Helper functions to move from canvas co-ords
// to chess co-ords and back
int
BoardView::convertToRank (int r)
{
  r = r / pieceSize;
  if (humanSide == sideWhite)
    r = 8 - r;
  else
    r++;
  return r;
}

int
BoardView::convertFromRank (int r)
{
  if (humanSide == sideWhite)
    r = 8 - r;
  else
    r--;
  return r * pieceSize;
}

char
BoardView::convertToFile (int f)
{
  f = f / pieceSize;
  if (humanSide == sideWhite)
    return f + 'a';
  else
    return 'h' - f;
}

int
BoardView::convertFromFile (char f)
{
  if (humanSide == sideWhite)
    f = f - 'a';
  else
    f = 'h' - f;
  return f * pieceSize;
}

// Pieces
Piece::Piece (QCanvas * canvas, int t):QCanvasRectangle (canvas)
{
  type = t;
  setSize (pieceSize, pieceSize);
  show ();
}

Piece *
BoardView::newPiece (int t, char f, int r)
{
  Piece *tmpPiece = new Piece (canvas (), t);
  movePiece (tmpPiece, f, r);
  list.append (tmpPiece);
  return tmpPiece;
}

void
BoardView::deletePiece (Piece * p)
{
  list.remove (p);
  canvas ()->update ();
}

void
BoardView::movePiece (Piece * p, char f, int r)
{
  p->rank = r;
  p->file = f;
  p->move (convertFromFile (f), convertFromRank (r));
  canvas ()->update ();
}

void
Piece::drawShape (QPainter & p)
{
  p.drawImage (int (x ()), int (y ()), *(imgList[type]));
}

void
Piece::buildImages (QImage theme)
{
  imgList.resize (12);
  int x;
  int y = 0;

  for (int j = 0; j < 2; j++)
    {
      x = 0;
      for (int i = 0; i < 6; i++)
	{
	  imgList.insert (i + (j * 6),
			  new QImage (theme.
				      copy (x, y, pieceSize, pieceSize)));
	  x += pieceSize;
	}
      y += pieceSize;
    }
}

// We hijack the movement::capturedPiece field
// for our own dark ends.. its now used for
// signalling which piece to promote to,
// and bool movement::promotion tells us whether
// to use it. movement::promotion will be used
// in rewinding history as well
void
BoardView::doMovement (movement * m)
{
  activeSide = !activeSide;

  // Promote
  if (m->promotion)
    {
      Piece *pc = findPiece (m->toFile, m->toRank);
      deletePiece (pc);
      newPiece (m->capturedPiece, m->toFile, m->toRank);
    };

  // Castle
  if (m->fromFile == 'e' &&
      (findPiece (m->fromFile, m->fromRank)->type == wKing ||
       findPiece (m->fromFile, m->fromRank)->type == bKing))
    {
      Piece *rook;
      movement *rookMove = new movement ();
      rookMove->promotion = FALSE;
      rookMove->fromRank = m->fromRank;
      rookMove->toRank = m->toRank;

      if (m->toFile == 'g')
	{
	  rook = findPiece ('h', m->fromRank);
	  if (rook)
	    {
	      rookMove->fromFile = 'h';
	      rookMove->toFile = 'f';
	      doMovement (rookMove);
	      activeSide = !activeSide;
	    }
	}
      else if (m->toFile == 'c')
	{
	  rook = findPiece ('a', m->fromRank);
	  if (rook)
	    {
	      rookMove->fromFile = 'a';
	      rookMove->toFile = 'd';
	      doMovement (rookMove);
	      activeSide = !activeSide;
	    }
	}
    }
  // Find any captured piece
  Piece *tmpPiece = findPiece (m->toFile, m->toRank);
  if (tmpPiece)
    {
      m->capturedPiece = tmpPiece->type;
      deletePiece (tmpPiece);
    }
  else
    m->capturedPiece = NONE;

  // Record the move
  history.push (m);

  // Move the piece
  tmpPiece = findPiece (m->fromFile, m->fromRank);
  movePiece (tmpPiece, m->toFile, m->toRank);

#ifdef CHESS_DEBUG
  qDebug ("after ::doMovement move count = %d\nactiveSide = %d",
	  history.count (), activeSide);
#endif
}

void
BoardView::decodeMovement (QString s, int p)
{
  movement *move = new movement;
  QChar tmp = s.at (0);
  move->fromFile = tmp.latin1 ();
  tmp = s.at (1);
  move->fromRank = tmp.digitValue ();
  tmp = s.at (2);
  move->toFile = tmp.latin1 ();
  tmp = s.at (3);
  move->toRank = tmp.digitValue ();

  move->capturedPiece = p;
  if (p != NONE)
    move->promotion = TRUE;
  else
    move->promotion = FALSE;

  doMovement (move);
}

void
BoardView::encodeMovement (movement * move)
{
  doMovement (move);

  QString moveS;
  moveS.append (move->fromFile);
  moveS.append (moveS.number (move->fromRank));
  moveS.append (move->toFile);
  moveS.append (moveS.number (move->toRank));
  if (move->promotion)
    {
      switch (move->capturedPiece)
	{
	case wQueen:
	case bQueen:
	  moveS.append ("Q");
	  break;
	case wKnight:
	case bKnight:
	  moveS.append ("N");
	  break;
	case wRook:
	case bRook:
	  moveS.append ("R");
	  break;
	case wBishop:
	case bBishop:
	  moveS.append ("B");
	  break;
	default:
	  qFatal ("Cant promote to this type!");
	  break;
	}
    }
  moveS.append ("\n");
  crafty->writeToStdin (moveS.latin1 ());
}

void
BoardView::readStdout ()
{
  QString input (QByteArray (crafty->readStdout ()));
#ifdef CHESS_DEBUG
  qDebug ("received this string from crafty->\n%s\n", input.latin1 ());
#endif

  const QString moveReg ("move [a-h][1-8][a-h][1-8]");
  const QString moveAndPromoteReg (moveReg + "[N|B|R|Q]");

  // Check for promotion
  QRegExp regExp (moveAndPromoteReg);
  int match = regExp.match (input, 0);
  if (match != -1)
    {
      QChar promotion = input.at (match + 9);
      qDebug ("promo = %c", promotion.latin1 ());
      int promotionType = NONE;
      switch (promotion.latin1 ())
	{
	case 'N':
	  promotionType = wKnight;
	  break;
	case 'R':
	  promotionType = wRook;
	  break;
	case 'B':
	  promotionType = wBishop;
	  break;
	case 'Q':
	  promotionType = wQueen;
	  break;
	default:
	  promotionType = NONE;
	};

      if (activeSide == sideBlack)
	promotionType = promotionType + 6;

      qDebug ("promoType = %d", promotionType);
      decodeMovement (input.mid (match + 5, 4), promotionType);
    }
  else
    {
      // Check for regular movement
      regExp.setPattern (moveReg);
      match = regExp.match (input, 0);
      if (match != -1)
	{
	  decodeMovement (input.mid (match + 5, 4), NONE);
	}
      // Check for an illegal move being signalled
      if (input.contains ("Illegal move:"))
	{
	  revertLastMove ();
	}
    }

  if (input.contains ("Black mates"))
    {
      playingGame = FALSE;
      emit (showMessage ("Black mates"));
    }
  else if (input.contains ("White mates"))
    {
      playingGame = FALSE;
      emit (showMessage ("White mates"));
// I dont know what the messages for these are yet :P
    }
  else if (input.contains (" resigns"))
    {
      playingGame = FALSE;
      emit (showMessage ("Computer resigns"));
    }
  else if (input.contains ("Draw"))
    {
      playingGame = FALSE;
      emit (showMessage ("Draw"));
    }
}

void
BoardView::emitErrorMessage ()
{
  qDebug ("emitting\n");
  if (activeSide != humanSide)
    emit (showMessage ("Not your move"));
  else
    emit (showMessage ("You are not playing a game"));
}

// This function only reverts -our- idea of the last move
// This is not useful outside of this widget
void
BoardView::revertLastMove ()
{
  activeSide = !activeSide;

  movement *lastMove = history.pop ();
#ifdef CHESS_DEBUG
  qDebug ("history shows to %c %d\n", lastMove->toFile, lastMove->toRank);
  qDebug ("history shows from %c %d\n", lastMove->fromFile,
	  lastMove->fromRank);
  qDebug ("captured = %d\n", lastMove->capturedPiece);
#endif

  // Move piece back
  Piece *tmpPiece = findPiece (lastMove->toFile, lastMove->toRank);
  movePiece (tmpPiece, lastMove->fromFile, lastMove->fromRank);

  // Add captured piece
  if (lastMove->capturedPiece != NONE)
    newPiece (lastMove->capturedPiece, lastMove->toFile, lastMove->toRank);

  // Uncastle.
  if (lastMove->fromFile == 'e' &&
      (tmpPiece->type == wKing ||
       tmpPiece->type == bKing) &&
      (lastMove->toFile == 'g' || lastMove->toFile == 'c'))
    revertLastMove ();

  // Unpromote
  if (lastMove->promotion)
    {
      if (activeSide == sideWhite)
	newPiece (wPawn, tmpPiece->file, tmpPiece->rank);
      else
	newPiece (bPawn, tmpPiece->file, tmpPiece->rank);
      deletePiece (tmpPiece);
    }
}

// This function reverts both ours and craftys last move
// This is useful for hooking up to a signal
void
BoardView::undo ()
{
  if ((humanSide == activeSide) && (history.count () > 1) && playingGame)
    {
      crafty->writeToStdin ("reset " +
			    QString::number (history.count () - 2));
      revertLastMove ();
      revertLastMove ();
    }
  else
    {
      if (history.count () < 2)
	emit (showMessage ("Make a move before you try to undo"));
      else
	emitErrorMessage ();
    }
}

void
BoardView::annotateGame ()
{
  emit (showMessage ("Annotating game"));
}

Piece *
BoardView::findPiece (char f, int r)
{
  QListIterator < Piece > it (list);
  Piece *tmpPiece;

  for (; it.current (); ++it)
    {
      tmpPiece = it.current ();
      if (tmpPiece->rank == r && tmpPiece->file == f)
	return tmpPiece;
    }
  return 0;
}

void
BoardView::newGame ()
{
  list.clear ();
  history.clear ();
  activeSide = 0;
  emit (showMessage (""));

  // At least the pawns are quick to setup
  for (int i = 0; i < 8; i++)
    {
      newPiece (wPawn, i + 'a', 2);
      newPiece (bPawn, i + 'a', 7);
    }

  for (int i = 0; i < 2; i++)
    {
      int j = i * 6;
      int k = i * 7;
      // rooks first
      newPiece (wRook + j, 'a', 1 + k);
      newPiece (wRook + j, 'h', 1 + k);
      newPiece (wKnight + j, 'b', 1 + k);
      newPiece (wBishop + j, 'c', 1 + k);
      newPiece (wQueen + j, 'd', 1 + k);
      newPiece (wKing + j, 'e', 1 + k);
      newPiece (wBishop + j, 'f', 1 + k);
      newPiece (wKnight + j, 'g', 1 + k);
    }

  canvas ()->update ();

  crafty->writeToStdin ("new\n");
  crafty->writeToStdin ("time 100/1\n");
  activeSide = sideWhite;
  if (humanSide == sideBlack)
    crafty->writeToStdin ("go\n");
}

void
BoardView::setTheme (QString filename)
{
  //QImage theme(filename + ".png");
  QImage theme = Resource::loadImage (QString ("chess/") + filename);
  pieceSize = theme.height () / 2;
#ifdef CHESS_DEBUG
  qDebug ("pieceSize = %d\n", pieceSize);
#endif
  Piece::pieceSize = pieceSize;

  setFrameStyle (QFrame::Plain);
  setFixedSize (8 * pieceSize, 8 * pieceSize);
  canvas ()->setBackgroundColor (Qt::red);
  canvas ()->resize (8 * pieceSize, 8 * pieceSize);

  QImage white = theme.copy (6 * pieceSize, 0, pieceSize, pieceSize);
  QImage black = theme.copy (6 * pieceSize, pieceSize, pieceSize, pieceSize);

  QPixmap bg = QPixmap (8 * pieceSize, 8 * pieceSize);
  QPainter p (&bg);

  bool col = FALSE;
  for (int i = 0; i < 8; i++)
    {
      for (int j = 0; j < 8; j++)
	{
	  QPoint point (i * pieceSize, j * pieceSize);
	  if (col)
	    {
	      p.drawImage (point, black);
	      col = FALSE;
	    }
	  else
	    {
	      p.drawImage (point, white);
	      col = TRUE;
	    }
	}
      col = !col;
    }
  canvas ()->setBackgroundPixmap (bg);

  Piece::buildImages (theme);

  canvas ()->update ();
}

// Board view widget
void
BoardView::contentsMousePressEvent (QMouseEvent * e)
{
  QCanvasItemList cList = canvas ()->collisions (e->pos ());
  if (activeSide == humanSide && playingGame)
    {
      if (!activePiece)
	{
	  if (cList.count ())
	    activePiece = (Piece *) (*(cList.at (0)));
	}
      else
	{
	  if (!(activePiece == (Piece *) (*(cList.at (0)))))
	    {
	      movement *move = new movement;
	      move->fromFile = activePiece->file;
	      move->fromRank = activePiece->rank;
	      move->toFile = convertToFile (e->pos ().x ());
	      move->toRank = convertToRank (e->pos ().y ());

	      if ((activePiece->type == wPawn && move->fromRank == 7
		   && move->toRank == 8) || (activePiece->type == bPawn
					     && move->fromRank == 2
					     && move->toRank == 1))
		{
		  // offer a promotion
		  emit (showMessage
			("you are meant to be offered a promotion here"));
		  move->promotion = TRUE;
		  move->capturedPiece = wQueen;
		}
	      else
		{
		  move->promotion = FALSE;
		  move->capturedPiece = NONE;
		}
	      encodeMovement (move);
	    }
	  activePiece = 0;
	}
    }
  else
    {
      emitErrorMessage ();
    }
}

void
BoardView::swapSides ()
{
  if (activeSide == humanSide && playingGame)
    {
      humanSide = !humanSide;

      int itemsInList = list.count ();
      Piece *tmpPiece;

      for (int c = 0; c < itemsInList; c++)
	{
	  tmpPiece = list.first ();
	  newPiece (tmpPiece->type, tmpPiece->file, tmpPiece->rank);
	  deletePiece (tmpPiece);
	}

      crafty->writeToStdin ("go\n");
    }
  else
    emitErrorMessage ();
}

BoardView::BoardView (QWidget * w, const char *name):
QCanvasView (0, w, name)
{
  humanSide = sideWhite;
  activeSide = sideWhite;
  playingGame = TRUE;

  activePiece = 0;
  list.setAutoDelete (TRUE);

  setCanvas (new QCanvas ());

  Config c ("Chess", Config::User);
  c.setGroup ("Theme");
  QString theme = c.readEntry ("imagefile", "simple-28");
  setTheme (theme);

  crafty = new QProcess (this);
  crafty->addArgument ("crafty");

  if (!crafty->start ())
    {
      // error handling
      QMessageBox::critical (0, tr ("Could not find crafty chess engine"),
			     tr ("Quit"));
      exit (-1);
    }

  connect (crafty, SIGNAL (readyReadStdout ()), this, SLOT (readStdout ()));
  connect (crafty, SIGNAL (processExited ()), this, SLOT (craftyDied ()));

  crafty->writeToStdin ("ics\n");
  crafty->writeToStdin ("kibitz=2\n");
  newGame ();
//    crafty->writeToStdin("xboard\n");
}
