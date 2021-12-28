#ifndef CHESS_H
#define CHESS_H

#include <qwidget.h>
#include <qcanvas.h>
#include <qmainwindow.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qstack.h>
#include <qvector.h>
#include <qprocess.h>

#define wPawn	0
#define	wKnight 1
#define wBishop 2
#define wRook 3
#define wQueen 4
#define wKing 5
#define bPawn 6
#define bKnight 7
#define bBishop 8
#define bRook 9
#define bQueen 10
#define bKing 11
#define NONE 12

#define sideWhite 0
#define sideBlack 1

struct movement {
    char fromFile;
    int fromRank;
    char toFile;
    int toRank;

    int capturedPiece;
		bool promotion;
};

class Piece:public QCanvasRectangle { 
 public:
    Piece( QCanvas *, int);
    ~Piece() {};

    static void buildImages(QImage);
    static int pieceSize;

    int rank;
    int file;
    int type;

 protected:
    void drawShape(QPainter &);

 private:
    static QVector < QImage > imgList;
};

class BoardView:public QCanvasView { 
    Q_OBJECT
				public:
    BoardView( QWidget *, const char *);
    ~BoardView(){};

 protected:
    void contentsMousePressEvent(QMouseEvent *);

signals:
	void showMessage(const QString &);

 public slots:
		void readStdout();
    void craftyDied() {
				qFatal("Crafty died unexpectedly\n");
    };
    void newGame();
    void setTheme(QString);
	void swapSides();
	void undo();
	void annotateGame();

 private:
    QProcess * crafty;
    QList < Piece > list;
    QStack < movement > history;
    Piece *activePiece;

    int pieceSize;

    void revertLastMove();
    void decodeMovement(QString, int);
	void encodeMovement(movement *);
    void doMovement(movement*);
	void emitErrorMessage();

    bool humanSide;
		bool activeSide;
		bool playingGame;

    int convertFromFile(char);
    int convertToRank(int);
    int convertFromRank(int);
    char convertToFile(int);

    Piece *findPiece(char f, int r);
    void movePiece(Piece *, char, int);
    Piece *newPiece( int, char, int);
		void deletePiece (Piece *);

};

#endif
