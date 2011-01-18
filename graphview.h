#ifndef GRAPHVIEW_H
#define GRAPHVIEW_H

#include <QAbstractItemView>
#include <QList>
#include <QPen>
#include <QBrush>
#include <QItemSelectionRange>

class GraphView : public QAbstractItemView
{
    Q_OBJECT
    Q_PROPERTY(int bars READ bars WRITE setBars)
    Q_PROPERTY(int spacing READ spacing WRITE setSpacing)
    Q_PROPERTY(int horizontalScale READ horizontalScale WRITE setHorizontalScale)
    Q_PROPERTY(QBrush selectionBrush READ selectionBrush WRITE setSelectionBrush);
public:
    explicit GraphView(QWidget *parent);

    int bars() const;
    int spacing() const;
    int horizontalScale() const;
    const QBrush & selectionBrush() const;

    virtual void setModel ( QAbstractItemModel * model );
    /**
      The following methods are pure virtual functions in QAbstractItemView and therefore
      have to be implemented in this class.
      */
    virtual QRect visualRect(const QModelIndex &index) const;
    virtual void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible);
    virtual QModelIndex indexAt(const QPoint &point) const;

signals:

public slots:
    void setBars(int value);
    void setSpacing(int value);
    void setHorizontalScale(int value);
    void setSelectionBrush(const QBrush &brush);

protected:
    /**
      The following methods are pure virtual functions in QAbstractItemView and therefore
      have to be implemented in this class.
      */
    virtual QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers);
    virtual int horizontalOffset() const;
    virtual int verticalOffset() const;
    virtual bool isIndexHidden(const QModelIndex &index) const;
    virtual void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command);
    virtual QRegion visualRegionForSelection(const QItemSelection &selection) const;

    virtual void paintEvent(QPaintEvent *p_event);
    virtual void updateGeometries();

    void getBounds(int row, int col, float &min, float&max);
    void getStatistics(int row, int col, float &mean, float &variance);

protected slots:
    virtual void rowsInserted ( const QModelIndex & parent, int start, int end );
    virtual void rowsRemoved ( const QModelIndex & parent, int start, int end );
    virtual void columnsInserted ( const QModelIndex & parent, int start, int end );
    virtual void columnsRemoved ( const QModelIndex & parent, int start, int end );

private:
    int bars_, spacing_, horizontalScale_;
    QList<QPen> pens;
    QBrush selectionBrush_;
};

#endif // GRAPHVIEW_H
