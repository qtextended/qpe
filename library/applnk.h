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
**********************************************************************/
#ifndef __APPLNK_H__
#define __APPLNK_H__

#include <qobject.h>
#include <qiconset.h>
#include <qlist.h>
#include <qstringlist.h>

class AppLnkSetPrivate;
class AppLnkPrivate;

class AppLnk
{
public:
    AppLnk();
    AppLnk( const QString &file );
    AppLnk( const AppLnk &copy ); // copy constructor
    virtual ~AppLnk();

    bool isValid() const { return !mLinkFile.isNull(); }

    QString name() const { return mName; }
    const QPixmap& pixmap() const;
    const QPixmap& bigPixmap() const;
    QString exec() const { return mExec; }
    QString type() const;
    QString rotation() const { return mRotation; }
    QString comment() const { return mComment; }
    QString file() const;
    QString linkFile() const;
    QStringList mimeTypes() const { return mMimeTypes; }
    QStringList categories() const { return mCategories; }
    int id() const { return mId; }

    void execute() const;
    void execute(const QStringList& args) const;
    void removeFiles();

    void setName( const QString& docname );
    void setFile( const QString& filename );
    void setLinkFile( const QString& filename );
    void setComment( const QString& comment );
    void setType( const QString& mimetype );
    void setIcon( const QString& iconname );
    void setCategories( const QStringList& );
    bool writeLink() const;

protected:
    QString mName;
    QPixmap mPixmap;
    QPixmap mBigPixmap;
    QString mExec;
    QString mType;
    QString mRotation;
    QString mComment;
    QString mFile;
    QString mLinkFile;
    QString mIconFile;
    QStringList mMimeTypes;
    QStringList mCategories;
    int mId;
    static int lastId;
    AppLnkPrivate *d;
    friend class AppLnkSet;
    
    virtual void invoke(const QStringList& args) const;
};

class DocLnk : public AppLnk
{
public:
    DocLnk();
    DocLnk( const DocLnk &o ) : AppLnk(o) { }
    DocLnk( const QString &file );
    virtual ~DocLnk();

protected:
    void invoke(const QStringList& args) const;
};

class AppLnkSet
{
public:
    AppLnkSet();
    AppLnkSet( const QString &dir );
    ~AppLnkSet();

    const AppLnk *find( int id ) const;

    QStringList types() const { return typs; }
    QString typeName( const QString& ) const;
    QPixmap typePixmap( const QString& ) const;
    QPixmap typeBigPixmap( const QString& ) const;

    void add(AppLnk*);
    bool remove(AppLnk*);

    const QList<AppLnk> &children() const { return mApps; }
    void detachChildren();

protected:
    friend class AppLnk;
    QList<AppLnk> mApps;
    QString mFile;
    QStringList typs;
    AppLnkSetPrivate *d;

private:
    AppLnkSet( const AppLnkSet & ); // no copying!
    void findChildren(const QString &, const QString& t, const QString& lt);
};

class DocLnkSet : public AppLnkSet
{
public:
    DocLnkSet( const QString &dir, const QString &mimefilter=QString::null );

    const QList<DocLnk> &children() const { return (const QList<DocLnk> &)mApps; }

    void appendFrom( DocLnkSet& other );

private:
    DocLnkSet( const DocLnkSet & ); // no copying!
    void findChildren(const QString &d, const QString& mimefilter, int);
};


#endif // __APPLNK_H__

