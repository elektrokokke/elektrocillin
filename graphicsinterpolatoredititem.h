#ifndef GRAPHICSINTERPOLATOREDITITEM_H
#define GRAPHICSINTERPOLATOREDITITEM_H

#include "interpolator.h"
#include "graphicsinterpolationitem.h"
#include "graphicsnodeitem.h"
#include <QObject>
#include <QGraphicsRectItem>
#include <QMenu>
#include <QPen>

class GraphicsInterpolatorEditSubItem;

class GraphicsInterpolatorEditItem : public QGraphicsRectItem
{
public:
    enum ControlPoint {
        FIRST = 0,
        LAST = 1
    };
    GraphicsInterpolatorEditItem(Interpolator *interpolator, const QRectF &rect, const QRectF &rectScaled, QGraphicsItem *parent = 0, int verticalGridCells = 8, int horizontalGridCells = 8, const QPen &nodePen = QPen(QBrush(qRgb(114, 159, 207)), 3), const QBrush &nodeBrush = QBrush(qRgb(52, 101, 164)));

    void setRect(const QRectF &rect, const QRectF &rectScaled);
    void setVisible(ControlPoint controlPoint, bool visible);
    void interpolatorChanged();
    Interpolator * getInterpolator();
protected:
    virtual void increaseControlPoints() = 0;
    virtual void decreaseControlPoints() = 0;
    virtual void changeControlPoint(int index, int nrOfControlPoints, double x, double y) = 0;
    friend class GraphicsInterpolatorEditSubItem;
private:
    Interpolator *interpolator;
    GraphicsInterpolatorEditSubItem *child;
    int verticalGridCells, horizontalGridCells;
    QPen nodePen;
    QBrush nodeBrush;
};

class GraphicsInterpolatorEditSubItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:

    GraphicsInterpolatorEditSubItem(Interpolator *interpolator, const QRectF &rect, const QRectF &rectScaled, GraphicsInterpolatorEditItem *parent, int verticalGridCells, int horizontalGridCells, const QPen &nodePen = QPen(QBrush(qRgb(114, 159, 207)), 3), const QBrush &nodeBrush = QBrush(qRgb(52, 101, 164)));

    void setRect(const QRectF &rect, const QRectF &rectScaled);
    void setVisible(GraphicsInterpolatorEditItem::ControlPoint controlPoint, bool visible);
    void interpolatorChanged();
    Interpolator * getInterpolator();

protected:
    virtual void mousePressEvent ( QGraphicsSceneMouseEvent * event );

private slots:
    void onIncreaseControlPoints();
    void onDecreaseControlPoints();
    void onNodePositionChangedScaled(QPointF position);

private:
    QRectF rectScaled;
    GraphicsInterpolatorEditItem *parent;
    QPen nodePen;
    QBrush nodeBrush;
    QMap<QObject*, int> mapSenderToControlPointIndex;
    QVector<GraphicsNodeItem*> nodes;
    Interpolator *interpolator;
    GraphicsInterpolationItem *interpolationItem;
    QMenu contextMenu;
    bool visible[2];

    GraphicsNodeItem * createNode(qreal x, qreal y, const QRectF &rectScaled);
};


#endif // GRAPHICSINTERPOLATOREDITITEM_H
