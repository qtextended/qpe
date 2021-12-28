
#include <qwidget.h>

class QLabel;
class GraphData;
class Graph;
class GraphLegend;
class FileSystem;

class StorageInfo : public QWidget
{
    Q_OBJECT
public:
    StorageInfo( QWidget *parent=0, const char *name=0 );
};

class MountInfo : public QWidget
{
    Q_OBJECT
public:
    MountInfo( const QString &path, const QString &ttl, QWidget *parent=0, const char *name=0 );
    ~MountInfo();

private slots:
    void updateData();

private:
    QString title;
    FileSystem *fs;
    QLabel *totalSize;
    GraphData *data;
    Graph *graph;
    GraphLegend *legend;
};

class FileSystem
{
public:
    FileSystem( const QString &p );

    void update();

    const QString &path() const { return fspath; }
    long blockSize() const { return blkSize; }
    long totalBlocks() const { return totalBlks; }
    long availBlocks() const { return availBlks; }
    bool mounted() const { return mnted; }

private:
    QString fspath;
    long blkSize;
    long totalBlks;
    long availBlks;
    bool mnted;
};


