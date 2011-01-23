#ifndef AUDIOBUFFERMODEL_H
#define AUDIOBUFFERMODEL_H

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
