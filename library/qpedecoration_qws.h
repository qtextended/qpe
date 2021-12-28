#ifndef QPE_DECORATION_QWS_H__
#define QPE_DECORATION_QWS_H__


#include <qwsdefaultdecoration_qws.h>
#include <qimage.h>
#include <qdatetime.h>
#include <qguardedptr.h>


#ifndef QT_NO_QWS_QPE_WM_STYLE

class QPEManager;
class QTimer;

class QPEDecoration : public QWSDefaultDecoration
{
public:
    QPEDecoration();
    virtual ~QPEDecoration();

    virtual QRegion region(const QWidget *, const QRect &rect, Region);
    virtual void paint(QPainter *, const QWidget *);
    virtual void paintButton(QPainter *, const QWidget *, Region, int state);

    void maximize( QWidget * );
    void minimize( QWidget * );
    virtual void help( QWidget * );

    enum QPERegion { Help=LastRegion+1 };
    void buttonClicked( QPERegion r );
    
protected:
    void fillTitle( QPainter *p, const QWidget *w, int x, int y, int w, int h );
//    virtual int getTitleWidth(const QWidget *);
    virtual int getTitleHeight(const QWidget *);
    virtual const char **menuPixmap();
    virtual const char **closePixmap();
    virtual const char **minimizePixmap();
    virtual const char **maximizePixmap();
    virtual const char **normalizePixmap();

protected:
    QImage imageOk;
    QImage imageClose;
    QImage imageHelp;
    QString helpFile;
    bool helpExists;
    QPEManager *qpeManager;
};


class QPEManager : public QObject
{
    Q_OBJECT
public:
    QPEManager( QPEDecoration *d, QObject *parent=0 );

    void updateActive();

protected:
    int pointInQpeRegion( QWidget *w, const QPoint &p );
    virtual bool eventFilter( QObject *, QEvent * );
    void drawButton( QWidget *w, QPEDecoration::QPERegion r, int state );

protected slots:
    void whatsThisTimeout();

protected:
    QPEDecoration *decoration;
    QGuardedPtr<QWidget> active;
    int helpState;
    QTime pressTime;
    QTimer *wtTimer;
};


#endif // QT_NO_QWS_QPE_WM_STYLE


#endif // QPE_DECORATION_QWS_H__
