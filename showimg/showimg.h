/****************************************************************************
** $Id: showimg.h,v 1.11 2001/08/31 00:18:30 martinj Exp $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef SHOWIMG_H
#define SHOWIMG_H

#include <qwidget.h>
#include <qmainwindow.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>


class QMenuBar;
class QPopupMenu;
class QWidgetStack;
class FileSelector;
class DocLnk;


class ImageWidget : public QWidget
{
    Q_OBJECT
public:
    ImageWidget( QWidget *parent=0 ) : QWidget( parent ) { }
    ~ImageWidget() { }

    void setPixmap( const QPixmap &pm ) { pixmap = pm; }

signals:
    void clicked();

protected:
    void paintEvent( QPaintEvent * );
    void mouseReleaseEvent(QMouseEvent* event);

private:
    QPixmap pixmap;
};


class ImagePane : public QWidget
{
    Q_OBJECT
public:
    ImagePane( QWidget *parent=0 );
    ~ImagePane() { }

    void showStatus();
    void hideStatus();
    QLabel  *statusLabel() { return status; }
    void setPixmap( const QPixmap &pm );

signals:
    void clicked();

private:
    ImageWidget	*image;
    QLabel	*status;
    QVBoxLayout *vb;

private slots:
    void imageClicked();
};


class ImageViewer : public QMainWindow
{
    Q_OBJECT
public:
    ImageViewer( QWidget *parent=0, const char *name=0, int wFlags=0 );
    ~ImageViewer();

    void loadImage( const char *fileName );
    void show(const QString& fileref);

protected:
    void resizeEvent( QResizeEvent * );
    void mousePressEvent( QMouseEvent * );
    void mouseMoveEvent( QMouseEvent * );
    void closeEvent( QCloseEvent * );

private:
    void updateCaption( QString name );
    bool loadSelected();
    void scale();
    void convertEvent( QMouseEvent* e, int& x, int& y );
    bool reconvertImage();
    int calcHeight();
    void setImage(const QImage& newimage);
    void updateStatus();

private slots:
    void setDocument(const QString& fileref);
    void doDelayedLoad();
    void openFile( const DocLnk &file );
    void open();
    void closeFileSelector();
    void hFlip();
    void vFlip();
    void rot180();
    void rot90();
    void normalView();
    void fullScreen();

private:
    QString filename;
    QString delayLoad;
    QImage image;			// the loaded image
    QPixmap pm;			// the converted pixmap
    QPixmap pmScaled;		// the scaled pixmap
    QMenuBar *menubar;
    ImagePane *imagePanel;
    QToolBar *toolBar;
    QWidgetStack *stack;
    FileSelector *fileSelector;
    int pickx, picky;
    int clickx, clicky;
    bool isFullScreen;
    bool bFromDocView; // a flag to indicate whether or not we were 
                          // launched from the document view...
};


#endif // SHOWIMG_H
