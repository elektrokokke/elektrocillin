#include "floattablemodel.h"

FloatTableModel::FloatTableModel(QObject *parent) :
    QAbstractTableModel(parent)
{
}

FloatTableModel::~FloatTableModel()
{
    clear();
}

void FloatTableModel::clear()
{
    int columns = columnCount();
    if (columns)
        beginRemoveColumns(QModelIndex(), 0, columns - 1);
    for (int i = 0; i < audioBuffers.size(); i++) {
        delete audioBuffers[i];
    }
    audioBuffers.clear();
    if (columns)
        endRemoveColumns();
}

QString FloatTableModel::getErrorString() const
{
    return errorString;
}

QVariant FloatTableModel::data ( const QModelIndex & index, int role ) const
{
    Q_ASSERT(index.column() < columnCount());
    Q_ASSERT(index.row() < rowCount());
    Q_ASSERT(index.row() < audioBuffers[index.column()]->size());
    if (role == Qt::DisplayRole) {
        return (*audioBuffers[index.column()])[index.row()];
    }
    return QVariant();
}

//QVariant AudioBufferModel::headerData ( int section, Qt::Orientation orientation, int role ) const
//{
//    if (role == Qt::DisplayRole) {
//        if (orientation == Qt::Vertical) {
//            if (section < rowCount())
//                return QAbstractTableModel::headerData(section, orientation, role);
//            // return a string representing the time of the given row, based on sample frequency:
//            QTime time(0, 0, 0, 0);
//            time = time.addMSecs((int)(section * 1000.0f / getFrequency()));
//            QString s = time.toString("HH:mm:ss.zzz");
//            return s;
//        }
//    }
//    return QAbstractTableModel::headerData(section, orientation, role);
//}

int FloatTableModel::rowCount ( const QModelIndex & parent ) const
{
    Q_ASSERT(!parent.isValid());
    if (audioBuffers.size())
        return audioBuffers[0]->size();
    else
        return 0;
}

int FloatTableModel::columnCount ( const QModelIndex & parent ) const
{
    Q_ASSERT(!parent.isValid());
    return audioBuffers.size();
}

bool FloatTableModel::insertRows ( int row, int count, const QModelIndex & parent )
{
    Q_ASSERT(!parent.isValid());
    Q_ASSERT(row >= 0);
    Q_ASSERT(row <= rowCount());
    beginInsertRows(parent, row, row + count - 1);
    for (int col = 0; col < columnCount(); col++) {
        audioBuffers[col]->insert(row, count, 0.0f);
    }
    endInsertRows();
    return true;
}

bool FloatTableModel::removeRows ( int row, int count, const QModelIndex & parent )
{
    Q_ASSERT(!parent.isValid());
    Q_ASSERT(row >= 0);
    Q_ASSERT(row + count <= rowCount());
    beginRemoveRows(parent, row, row + count - 1);
    for (int col = 0; col < columnCount(); col++) {
        audioBuffers[col]->remove(row, count);
    }
    endRemoveRows();
    return true;
}

bool FloatTableModel::insertColumns ( int column, int count, const QModelIndex & parent )
{
    Q_ASSERT(!parent.isValid());
    Q_ASSERT(column >= 0);
    Q_ASSERT(column <= columnCount());
    beginInsertColumns(parent, column, column + count - 1);
    int rows = rowCount();
    audioBuffers.insert(column, count, 0);
    for (int i = column; i < column + count; i++) {
        audioBuffers[i] = new QVector<float>(rows);
    }
    endInsertColumns();
    return true;
}

bool FloatTableModel::removeColumns ( int column, int count, const QModelIndex & parent )
{
    Q_ASSERT(!parent.isValid());
    Q_ASSERT(column >= 0);
    Q_ASSERT(column + count <= columnCount());
    beginRemoveColumns(parent, column, column + count - 1);
    for (int i = column; i < column + count; i++) {
        delete audioBuffers[i];
    }
    audioBuffers.remove(column, count);
    endRemoveColumns();
    return true;
}
