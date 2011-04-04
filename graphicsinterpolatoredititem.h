#ifndef GRAPHICSINTERPOLATOREDITITEM_H
#define GRAPHICSINTERPOLATOREDITITEM_H

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

#include "interpolator.h"
#include "graphicsinterpolationitem.h"
#include "graphicsnodeitem.h"
#include "graphicslabelitem.h"
#include <QObject>
#include <QGraphicsRectItem>
#include <QMenu>
#include <QPen>
#include <QFont>
#include <QMap>
#include <QGraphicsSimpleTextItem>

class GraphicsInterpolatorGraphItem;

class GraphicsInterpolatorEditItem : public QGraphicsRectItem
{
public:
    enum ControlPoint {
        FIRST = 0,
        LAST = 1
    };
    GraphicsInterpolatorEditItem(Interpolator *interpolator, const QRectF &rect, const QRectF &rectScaled, QGraphicsItem *parent = 0, int verticalSlices = 8, int horizontalSlices = 8, bool logarithmicX = false);

    void setRect(const QRectF &rect, const QRectF &rectScaled);
    void setVisible(ControlPoint controlPoint, bool visible);
    void interpolatorChanged();
    Interpolator * getInterpolator();
    QRectF getInnerRectangle() const;
    GraphicsInterpolatorGraphItem * getGraphItem();
protected:
    virtual void increaseControlPoints() = 0;
    virtual void decreaseControlPoints() = 0;
    virtual void changeControlPoint(int index, double x, double y) = 0;
    friend class GraphicsInterpolatorGraphItem;
private:
    Interpolator *interpolator;
    GraphicsInterpolatorGraphItem *child;
    int verticalSlices, horizontalSlices;
    bool logarithmicX;
    QFont font;
};

class GraphicsInterpolatorGraphItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:

    GraphicsInterpolatorGraphItem(Interpolator *interpolator, const QRectF &rect, const QRectF &rectScaled, GraphicsInterpolatorEditItem *parent, bool logarithmicX);

    void setRect(const QRectF &rect, const QRectF &rectScaled);
    void setVisible(GraphicsInterpolatorEditItem::ControlPoint controlPoint, bool visible);
    void interpolatorChanged();
    Interpolator * getInterpolator();

    void setNodePen(const QPen &pen);
    const QPen & getNodePen() const;
    void setNodeBrush(const QBrush &brush);
    const QBrush & getNodeBrush() const;

protected:
    virtual void mousePressEvent ( QGraphicsSceneMouseEvent * event );

private slots:
    void onIncreaseControlPoints();
    void onDecreaseControlPoints();
    void onNodePositionChangedScaled(QPointF position);

private:
    QRectF rectScaled;
    GraphicsInterpolatorEditItem *parent;
    QPen nodePen, nodePenNamed;
    QBrush nodeBrush, nodeBrushNamed;
    QMap<QObject*, int> mapSenderToControlPointIndex;
    QVector<GraphicsNodeItem*> nodes;
    QVector<GraphicsLabelItem*> nodeLabels;
    Interpolator *interpolator;
    GraphicsInterpolationItem *interpolationItem;
    QMenu contextMenu;
    bool visible[2];
    QFont font;
    bool logarithmicX;

    GraphicsNodeItem * createNode(qreal x, qreal y, const QRectF &rectScaled);
};


#endif // GRAPHICSINTERPOLATOREDITITEM_H
