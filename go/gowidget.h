/****************************************************************************
** $Id: gowidget.h,v 1.5 2001/06/05 07:05:08 warwick Exp $
**
** Definition of something or other
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef GOWIDGET_H
#define GOWIDGET_H

#include <qmainwindow.h>
#include "amigo.h"


class QToolBar;

class GoMainWidget : public QMainWindow
{
    Q_OBJECT
public:
    GoMainWidget( QWidget *parent=0, const char* name=0);
protected:
    void resizeEvent( QResizeEvent * );
private:
    QToolBar *toolbar;
    
};


class QLabel;
class GoWidget : public QWidget
{
    Q_OBJECT
public:
    GoWidget( QWidget *parent=0, const char* name=0);
    ~GoWidget();

    void doMove( int x, int y );
    void doComputerMove();

    void readConfig();
    void writeConfig();

public slots:    
    void pass();
    void resign();
    void newGame();
    void setTwoplayer( bool );
    void setHandicap( int );
signals:
    void showScore( const QString& );
    void showTurn( const QPixmap& );
    
protected:
    void paintEvent( QPaintEvent * );
    void mousePressEvent( QMouseEvent * );
    void mouseMoveEvent( QMouseEvent * );
    void mouseReleaseEvent( QMouseEvent * );
    void resizeEvent( QResizeEvent * );
private:
    void init();
    void removeStone(short x, short y);
    void placeStone (enum bVal c, short x, short y );

    void refresh( int x, int y );
    void showStone( int x, int y, enum bVal );
    void reportPrisoners(int,int);
    
    inline int x2board( int x ) { return (x-bx+d/2)/d; }
    inline int y2board( int y ) { return (y-by+d/2)/d; }

    void endGame();

    bool twoplayer;
    enum bVal currentPlayer;
    bool gameActive;
    int nPassed;
    signed char board[19][19];

    int d; //distance between lines
    int bx; //vertical baseline
    int by; //horizontal baseline
    
    int lastX,lastY;
    int newX,newY;
    
    static GoWidget *self;

    friend void removestone(short x, short y);
    friend void	intrPrisonerReport( short, short );
    friend void placestone(enum bVal c, short x, short y );
};





#endif
