#ifndef AUDIOBUFFERMODEL_H
#define AUDIOBUFFERMODEL_H

/*
    Copyright 2011 Arne Jacobs <jarne@jarne.de>

    This file is part of elektrocillin.

    Elektrocillin is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QAbstractTableModel>
#include <QVector>

class FloatTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit FloatTableModel(QObject *parent = 0);
    virtual ~FloatTableModel();

    bool loadSawTooth(float durationInSeconds, float frequency);
    QString getErrorString() const;

    void clear();

    virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    virtual bool setData ( const QModelIndex &index, const QVariant &value, int role );
//    virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    virtual int columnCount ( const QModelIndex & parent = QModelIndex() ) const;

    virtual bool insertRows ( int row, int count, const QModelIndex & parent = QModelIndex() );
    virtual bool removeRows ( int row, int count, const QModelIndex & parent = QModelIndex() );
    virtual bool insertColumns ( int column, int count, const QModelIndex & parent = QModelIndex() );
    virtual bool removeColumns ( int column, int count, const QModelIndex & parent = QModelIndex() );

signals:

protected:
    QVector<QVector<float>*> audioBuffers;
private:
    QString errorString;
};

#endif // AUDIOBUFFERMODEL_H
