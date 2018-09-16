
/****************************************************************************
** Copyright (c) 2006 - 2011, the LibQxt project.
** See the Qxt AUTHORS file for a list of authors and copyright holders.
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in the
**       documentation and/or other materials provided with the distribution.
**     * Neither the name of the LibQxt project nor the
**       names of its contributors may be used to endorse or promote products
**       derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
** DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
** (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
** LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
** ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
** <http://libqxt.org>  <foundation@libqxt.org>
*****************************************************************************/

#ifndef QxtBdbHash_H_kpasd
#define QxtBdbHash_H_kpasd

#include "qxtbdb.h"
#include <QBuffer>
#include <QDataStream>
#include <QVariant>
#include <qxtsharedprivate.h>
#include <qxtglobal.h>


template<class KEY, class VAL>
class QxtBdbHashIterator;

template<class KEY, class VAL>
class /*QXT_BERKELEY_EXPORT*/ QxtBdbHash
{
public:
    QxtBdbHash();
    QxtBdbHash(QString file);
    bool open(QString file);

    QxtBdbHashIterator<KEY, VAL> begin();
    QxtBdbHashIterator<KEY, VAL> end();
    QxtBdbHashIterator<KEY, VAL> find(const KEY & key);

    void clear();
    bool contains(const KEY & key) const;
    bool remove(const KEY & key);
    bool insert(KEY k, VAL v);
    const VAL value(const KEY & key) const;
    const VAL operator[](const KEY & key) const;

    bool flush();

private:
    int meta_id_key;
    int meta_id_val;
    QxtSharedPrivate<QxtBdb> qxt_d;
};


class QxtBdbHashIteratorPrivate
{
public:
    QxtBdb * db;
    BerkeleyDB::DBC *dbc;
    void invalidate()
    {
        if (dbc)
            dbc->c_close(dbc);
        dbc = 0;
    }
    ~QxtBdbHashIteratorPrivate()
    {
        invalidate();
    }
};

template<class KEY, class VAL>
class QxtBdbHashIterator
{
public:
    QxtBdbHashIterator();
    QxtBdbHashIterator(const QxtBdbHashIterator<KEY, VAL> & other);
    QxtBdbHashIterator & operator= (const QxtBdbHashIterator<KEY, VAL> & other);

    bool isValid() const;

    operator KEY() const;

    KEY     key() const;
    VAL   value() const;

    QxtBdbHashIterator<KEY, VAL>    operator + (int j) const;
    QxtBdbHashIterator<KEY, VAL> &  operator ++ ();
    QxtBdbHashIterator<KEY, VAL>    operator ++ (int);
    QxtBdbHashIterator<KEY, VAL> &  operator += (int j);

    QxtBdbHashIterator<KEY, VAL>    operator - (int j) const;
    QxtBdbHashIterator<KEY, VAL> &  operator -- ();
    QxtBdbHashIterator<KEY, VAL>    operator -- (int);
    QxtBdbHashIterator<KEY, VAL> &  operator -= (int j);



    QxtBdbHashIterator<KEY, VAL> erase();


private:
    friend class QxtBdbHash<KEY, VAL>;
    QxtBdbHashIterator(BerkeleyDB::DBC*, QxtBdb * p);
    QxtSharedPrivate<QxtBdbHashIteratorPrivate> qxt_d;

    int meta_id_key;
    int meta_id_val;

    /*won't work. no support in bdb*/
    bool operator== (const QxtBdbHashIterator<KEY, VAL> & other) const
    {
        return false;
    }
    bool operator!= (const QxtBdbHashIterator<KEY, VAL> & other) const
    {
        return false;
    }
};

















template<class KEY, class VAL>
QxtBdbHash<KEY, VAL>::QxtBdbHash()
{
    qxt_d = new QxtBdb();

    ///this will fail to compile if KEY or VALUE are not declared metatypes. Good.
    meta_id_key = qMetaTypeId<KEY>();
    meta_id_val = qMetaTypeId<VAL>();
}


template<class KEY, class VAL>
QxtBdbHash<KEY, VAL>::QxtBdbHash(QString file)
{
    qxt_d = new QxtBdb();
    open(file);

    ///this will fail to compile if KEY or VALUE are not declared metatypes. Good.
    meta_id_key = qMetaTypeId<KEY>();
    meta_id_val = qMetaTypeId<VAL>();
}

template<class KEY, class VAL>
bool QxtBdbHash<KEY, VAL>::open(QString file)
{
    return qxt_d().open(file, QxtBdb::CreateDatabase | QxtBdb::LockFree);
}




template<class KEY, class VAL>
QxtBdbHashIterator<KEY, VAL> QxtBdbHash<KEY, VAL>::begin()
{
    BerkeleyDB::DBC *cursor;
    qxt_d().db->cursor(qxt_d().db, NULL, &cursor, 0);
    if (qxt_d().get((void*)0, 0, 0, 0, DB_FIRST, cursor))
        return QxtBdbHashIterator<KEY, VAL>(cursor, &qxt_d());
    else
        return QxtBdbHashIterator<KEY, VAL>();
}

template<class KEY, class VAL>
QxtBdbHashIterator<KEY, VAL> QxtBdbHash<KEY, VAL>::end()
{
    BerkeleyDB::DBC *cursor;
    qxt_d().db->cursor(qxt_d().db, NULL, &cursor, 0);
    if (qxt_d().get((void*)0, 0, 0, 0, DB_LAST, cursor))
        return QxtBdbHashIterator<KEY, VAL>(cursor, &qxt_d());
    else
        return QxtBdbHashIterator<KEY, VAL>();
}

template<class KEY, class VAL>
QxtBdbHashIterator<KEY, VAL> QxtBdbHash<KEY, VAL>::find(const KEY & k)
{
    BerkeleyDB::DBC *cursor;
    qxt_d().db->cursor(qxt_d().db, NULL, &cursor, 0);
    if (qxt_d().get(&k, meta_id_key, 0, 0, DB_SET, cursor))
        return QxtBdbHashIterator<KEY, VAL>(cursor, &qxt_d());
    else
        return QxtBdbHashIterator<KEY, VAL>();
}






template<class KEY, class VAL>
void QxtBdbHash<KEY, VAL>::clear()
{
    if (!qxt_d().isOpen)
        return;

    BerkeleyDB::u_int32_t x;
    qxt_d().db->truncate(qxt_d().db, NULL, &x, 0);

}


template<class KEY, class VAL>
bool QxtBdbHash<KEY, VAL>::contains(const KEY & k) const
{
    if (!qxt_d().isOpen)
        return false;

    BerkeleyDB::DBT key;
    /* Zero out the DBTs before using them. */
    memset(&key, 0, sizeof(BerkeleyDB::DBT));

    QByteArray d_key;
    {
        QBuffer buffer(&d_key);
        buffer.open(QIODevice::WriteOnly);
        QDataStream s(&buffer);
        if (!QMetaType::save(s, meta_id_key, &k))
            qCritical("QMetaType::save failed. is your type registered with the QMetaType?");

        buffer.close();
    }

    key.data = d_key.data();
    key.size = d_key.size();

    return (qxt_d().db->exists(qxt_d().db, NULL, &key, 0) == 0);
}

template<class KEY, class VAL>
bool QxtBdbHash<KEY, VAL>::remove(const KEY & k)
{
    if (!qxt_d().isOpen)
        return false;

    BerkeleyDB::DBT key;
    /* Zero out the DBTs before using them. */
    memset(&key, 0, sizeof(BerkeleyDB::DBT));

    QByteArray d_key;
    {
        QBuffer buffer(&d_key);
        buffer.open(QIODevice::WriteOnly);
        QDataStream s(&buffer);
        if (!QMetaType::save(s, meta_id_key, &k))
            qCritical("QMetaType::save failed. is your type registered with the QMetaType?");

        buffer.close();
    }

    key.data = d_key.data();
    key.size = d_key.size();


    return (qxt_d().db->del(qxt_d().db, NULL, &key, 0) == 0);
}



template<class KEY, class VAL>
bool QxtBdbHash<KEY, VAL>::insert(KEY k, VAL v)
{
    if (!qxt_d().isOpen)
        return false;

    QByteArray d_key, d_value;

    {
        QBuffer buffer(&d_key);
        buffer.open(QIODevice::WriteOnly);
        QDataStream s(&buffer);
        if (!QMetaType::save(s, meta_id_key, &k))
            qCritical("QMetaType::save failed. is your type registered with the QMetaType?");

        buffer.close();
    }
    {
        QBuffer buffer(&d_value);
        buffer.open(QIODevice::WriteOnly);
        QDataStream s(&buffer);
        if (!QMetaType::save(s, meta_id_val, &v))
            qCritical("QMetaType::save failed. is your value registered with the QMetaType?");

        buffer.close();
    }

    BerkeleyDB::DBT key, value;
    /* Zero out the DBTs before using them. */
    memset(&key, 0,  sizeof(BerkeleyDB::DBT));
    memset(&value, 0, sizeof(BerkeleyDB::DBT));

    key.data = d_key.data();
    key.size = d_key.size();

    value.data = d_value.data();
    value.size = d_value.size();


    int ret = qxt_d().db->put(qxt_d().db, NULL, &key, &value, 0);
    return (ret == 0);

}



template<class KEY, class VAL>
const VAL QxtBdbHash<KEY, VAL>::value(const KEY & k) const
{
    if (!qxt_d().isOpen)
        return VAL() ;

    VAL v;
    if (!qxt_d().get(&k, meta_id_key, &v, meta_id_val))
        return VAL();
    return v;
}

template<class KEY, class VAL>
const VAL QxtBdbHash<KEY, VAL>::operator[](const KEY & key) const
{
    return value(key);
}


template<class KEY, class VAL>
bool QxtBdbHash<KEY, VAL>::flush()
{
    return qxt_d().flush();
}
















template<class KEY, class VAL>
QxtBdbHashIterator<KEY, VAL>::QxtBdbHashIterator()
{
    qxt_d = new QxtBdbHashIteratorPrivate;
    qxt_d().dbc = 0;
    qxt_d().db = 0;
    meta_id_key = qMetaTypeId<KEY>();
    meta_id_val = qMetaTypeId<VAL>();
}

template<class KEY, class VAL>
QxtBdbHashIterator<KEY, VAL>::QxtBdbHashIterator(const QxtBdbHashIterator<KEY, VAL> & other)
{
    ///FIXME: possible leaking, since the other isnt properly destructed?
    qxt_d = other.qxt_d;
    meta_id_key = qMetaTypeId<KEY>();
    meta_id_val = qMetaTypeId<VAL>();

}

template<class KEY, class VAL>
QxtBdbHashIterator<KEY, VAL> & QxtBdbHashIterator<KEY, VAL>::operator= (const QxtBdbHashIterator<KEY, VAL> & other)
{
    ///FIXME: possible leaking, since the other isnt properly destructed?
    qxt_d = other.qxt_d;
    return *this;
}

template<class KEY, class VAL>
bool QxtBdbHashIterator<KEY, VAL>::isValid() const
{
    return (qxt_d().dbc != 0);
}


template<class KEY, class VAL>
QxtBdbHashIterator<KEY, VAL>::operator KEY() const
{
    return key();
}


template<class KEY, class VAL>
KEY     QxtBdbHashIterator<KEY, VAL>::key() const
{

    if (!isValid())
        return KEY();
    KEY k;
    if (qxt_d().db->get(&k, meta_id_key, 0, 0, DB_CURRENT, qxt_d().dbc))
        return k;
    else
        return KEY();
}


template<class KEY, class VAL>
VAL   QxtBdbHashIterator<KEY, VAL>::value() const
{
    if (!isValid())
        return VAL();

    VAL v;
    if (qxt_d().db->get((void*)0, 0, &v, meta_id_val, DB_CURRENT, qxt_d().dbc))
        return v;
    else
        return VAL();
}


template<class KEY, class VAL>
QxtBdbHashIterator<KEY, VAL>    QxtBdbHashIterator<KEY, VAL>::operator + (int j) const
{
    // if j is negative, it should subtract as apposed to do nothing...
    if(j < 0)
        return this->operator-(j*(-1));

    QxtBdbHashIterator<KEY, VAL> d = *this;
    for (int i = 0;i < j;i++)
        ++d;
    return d;
}

template<class KEY, class VAL>
QxtBdbHashIterator<KEY, VAL> &  QxtBdbHashIterator<KEY, VAL>::operator ++ () /*prefix*/
{
    if (!isValid())
        return *this;

    if (!qxt_d().db->get((void*)0, 0, 0, 0, DB_NEXT, qxt_d().dbc))
    {
        qxt_d().invalidate();
    }
    return *this;
}

template<class KEY, class VAL>
QxtBdbHashIterator<KEY, VAL>   QxtBdbHashIterator<KEY, VAL>::operator ++ (int) /*postfix*/
{
    if (!isValid())
        return *this;

    QxtBdbHashIterator<KEY, VAL> d = *this;
    this->operator++();
    return d;
}

template<class KEY, class VAL>
QxtBdbHashIterator<KEY, VAL> &  QxtBdbHashIterator<KEY, VAL>::operator += (int j)
{
    // if j is negative it should subtract as apposed to do nothing
    if(j < 0)
        return this->operator-=(j*(-1));

    for (int i = 0;i < j;i++)
        this->operator++();
    return *this;
}


template<class KEY, class VAL>
QxtBdbHashIterator<KEY, VAL>    QxtBdbHashIterator<KEY, VAL>::operator - (int j) const
{
    // if j is negative, then we should add as opposed to do noting... I really need to learn to spell "opposed" correctly more regularly... hmm..
    if(j < 0)
        return this->operator+(j*(-1));

    QxtBdbHashIterator<KEY, VAL> d = *this;
    for (int i = 0;i < j;i++)
        --d;
    return d;
}

template<class KEY, class VAL>
QxtBdbHashIterator<KEY, VAL> &  QxtBdbHashIterator<KEY, VAL>::operator -- ()  /*prefix*/
{
    if (!isValid())
        return *this;

    if (!qxt_d().db->get((void*)0, 0, 0, 0, DB_PREV, qxt_d().dbc))
        qxt_d().invalidate();

    return *this;

}

template<class KEY, class VAL>
QxtBdbHashIterator<KEY, VAL>   QxtBdbHashIterator<KEY, VAL>::operator -- (int) /*postfix*/
{
    if (!isValid())
        return *this;

    QxtBdbHashIterator<KEY, VAL> d = *this;
    this->operator--();
    return d;
}

template<class KEY, class VAL>
QxtBdbHashIterator<KEY, VAL> &  QxtBdbHashIterator<KEY, VAL>::operator -= (int j)
{
    // if j is negative, we should add
    if(j < 0)
        return this->operator+=(j*(-1));

    for (int i = 0;i < j;i++)
        this->operator--();
    return *this;
}




template<class KEY, class VAL>
QxtBdbHashIterator<KEY, VAL> QxtBdbHashIterator<KEY, VAL>::erase()
{
    BerkeleyDB::DBC * newdbc;
    qxt_d().dbc->c_dup(qxt_d().dbc, &newdbc, DB_POSITION);
    QxtBdbHashIterator<KEY, VAL> d(newdbc, qxt_d().db);
    qxt_d().dbc->del(qxt_d().dbc, NULL);
    ++d;

    qxt_d().invalidate();
    return d;
}

template<class KEY, class VAL>
QxtBdbHashIterator<KEY, VAL>::QxtBdbHashIterator(BerkeleyDB::DBC* dbc, QxtBdb * p)
{
    qxt_d = new QxtBdbHashIteratorPrivate;
    qxt_d().dbc = dbc;
    qxt_d().db = p;
    meta_id_key = qMetaTypeId<KEY>();
    meta_id_val = qMetaTypeId<VAL>();
}








#endif

