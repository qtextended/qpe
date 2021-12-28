
#include <qtabwidget.h>
#include <qlist.h>
#include "qimpenchar.h"
#include "qimpenprofile.h"

class QListBox;
class QLabel;
class QComboBox;
class QPushButton;
class QTimer;
class QIMPenWidget;
class QIMPenMatch;

class HandwritingHelp : public QTabWidget
{
    Q_OBJECT
public:
    HandwritingHelp( QIMPenProfile *p, QWidget *parent=0, const char *name=0, WFlags f=0 );

protected:
    virtual void showEvent( QShowEvent * );
    virtual void hideEvent( QHideEvent * );
};

class HandwritingTrainer : public QWidget
{
    Q_OBJECT
public:
    HandwritingTrainer( QIMPenProfile *p, QWidget *parent=0, const char *name=0 );
    ~HandwritingTrainer();

private slots:
    void selectChar( int );
    void selectCharSet( int );
    void noMatch();
    void matched( const QIMPenCharMatchList &ml );
    void prevChar();
    void nextChar();
    void redrawChar();
    void beginStroke();
    void strokeEntered( QIMPenStroke * );

private:
    virtual void showEvent( QShowEvent * );
    QString rateString( int rate ) const;
    void setCurrentChar( QIMPenChar *c );
    void fillCharList();
    QIMPenChar *findPrev();
    QIMPenChar *findNext();

private:
    QIMPenMatch *matcher;
    QIMPenCharSet *currentSet;
    QIMPenChar *currentChar;
    QIMPenProfile *profile;
    uint currentCode;
    QIMPenWidget *refPw;
    QIMPenWidget *pracPw;
    QComboBox *charSetCombo;
    QListBox *charList;
    QLabel *result;
    QPushButton *prevBtn;
    QPushButton *nextBtn;
    QTimer *redrawTimer;
};

