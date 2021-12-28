#ifndef CATEGORYLIST_H
#define CATEGORYLIST_H

#include <qlistview.h>

class CategoryListPrivate;

class CategoryList : public QListView
{ 
    Q_OBJECT

public:
    CategoryList( QWidget* parent=0, const char* name=0 );
    ~CategoryList();

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    void setCheckedList( const QStringList& );
    QStringList checkedList() const;

public slots:
    void addItem(const QString& category);
    void removeItem(const QString& category);

private:
    CategoryListPrivate* d;
    void updateCategories();
    void addItemDirect(const QString& category, bool checked);
};

#endif // CATEGORYLIST_H
