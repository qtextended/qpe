#ifndef LNKPROPERTIES_H
#define LNKPROPERTIES_H
#include "lnkpropertiesbase.h"

class AppLnk;
class QListViewItem;

class LnkProperties : public LnkPropertiesBase
{ 
    Q_OBJECT

public:
    LnkProperties( AppLnk* lnk, QWidget* parent = 0 );
    ~LnkProperties();

    void done(int);

private slots:
    void openLnk();
    void unlinkLnk();
    void duplicateLnk();
    void addCat();
    void delCat();
    void selCat(QListViewItem*);

private:
    AppLnk* lnk;
};

#endif // LNKPROPERTIES_H
