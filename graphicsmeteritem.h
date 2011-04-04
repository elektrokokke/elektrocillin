#ifndef GRAPHICSMETERITEM_H
#define GRAPHICSMETERITEM_H

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

#include "graphicslabelitem.h"
#include "graphicsnodeitem.h"
#include <QGraphicsPathItem>
#include <QGraphicsLineItem>
#include <QPen>
#include <QBrush>

class GraphicsMeterItem : public QObject, public QGraphicsPathItem
{
    Q_OBJECT
public:
    enum EllipsisHalf {
        TOP_HALF,
        BOTTOM_HALF
    };
    GraphicsMeterItem(const QRectF &rect, const QString &name, double minValue, double maxValue, double value, int slices, int valuesPerSlice = 10, EllipsisHalf half = TOP_HALF, QGraphicsItem *parent = 0);

    void setRange(double minValue, double maxValue, double value, int slices);

public slots:
    void setValue(double value);
signals:
    void valueChanged(double value);
private slots:
    void onNodePositionChangedScaled(QPointF pos);
private:
    QString name;
    double minValue, maxValue, value;
    int slices, valuesPerSlice;
    EllipsisHalf half;
    int verticalPadding;
    QPen nodePen;
    QBrush nodeBrush;
    QRectF rect, innerRect;
    QPointF innerCenter;
    GraphicsNodeItem *nodeItem;
    GraphicsLabelItem *labelItem;
    QGraphicsLineItem *needleItem;
    QVector<QGraphicsItem*> ticks;

    void initTicks();
};

#endif // GRAPHICSMETERITEM_H
