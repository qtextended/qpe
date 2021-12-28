#ifndef PACKAGEMANAGER_H
#define PACKAGEMANAGER_H
#include "packagemanagerbase.h"
#include <qintdict.h>

class PackageItem;
class PackageDetails;
class PackageManagerSettings;

class PackageManager : public PackageManagerBase
{ 
    Q_OBJECT

public:
    PackageManager( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~PackageManager();

public slots:
    void doDetails();
    void doSettings();
    void doFind();
    void doUpgrade();
    void done(int);

private slots:
    void installCurrent();
    void removeCurrent();
    void doNextDetails();

    void newServer();
    void editServer(int);
    void removeServer();
    void nameChanged(const QString&);
    void urlChanged(const QString&);

private:
    void startMultiRun(int jobs);
    int runIpkg(const QString& args);

    bool readIpkgConfig(const QString& conffile);

    void doCurrentDetails(bool);
    void updatePackageList();
    PackageItem* current() const;
    QString fullDetails(const QString& pk);

    QStringList findPackages( const QRegExp& re );
    void selectPackages( const QStringList& l );

    bool commitWithIpkg();

    PackageManagerSettings* settings;
    QIntDict<QString> serverurl;
    int editedserver;
    int ipkg_old;
    void writeSettings();
    void readSettings();

    PackageDetails* details;
};

#endif // PACKAGEMANAGER_H
