#ifndef GRAPHICSINTERPOLATOREDITITEM_H
#define GRAPHICSINTERPOLATOREDITITEM_H

#include "interpolator.h"
#include "graphicsinterpolationitem.h"
#include "graphicsnodeitem.h"
#include <QObject>
#include <QGraphicsRectItem>
#include <QMenu>

class GraphicsInterpolatorEditItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    GraphicsInterpolatorEditItem(Interpolator *interpolator, const QRectF &rect, const QRectF &rectScaled, QGraphicsItem *parent = 0);

protected:
    virtual void increaseControlPoints() = 0;
    virtual void decreaseControlPoints() = 0;
    virtual void changeControlPoint(int index, int nrOfControlPoints, double x, double y) = 0;
    virtual void mousePressEvent ( QGraphicsSceneMouseEvent * event );

private slots:
    void onIncreaseControlPoints();
    void onDecreaseControlPoints();
    void onNodePositionChangedScaled(QPointF position);

private:
    QRectF rectScaled;
    QMap<QObject*, int> mapSenderToControlPointIndex;
    QVector<GraphicsNodeItem*> nodes;
    Interpolator *interpolator;
    GraphicsInterpolationItem *interpolationItem;
    QMenu contextMenu;

    GraphicsNodeItem * createNode(qreal x, qreal y, const QRectF &rectScaled);
};


#endif // GRAPHICSINTERPOLATOREDITITEM_H
