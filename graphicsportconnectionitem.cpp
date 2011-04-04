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

#include "graphicsportconnectionitem.h"
#include <QBrush>
#include <QPen>

GraphicsPortConnectionItem::GraphicsPortConnectionItem(const QString &port1_, const QString &port2_, QGraphicsScene *scene) :
    QGraphicsPathItem(0, scene),
    port1(port1_),
    port2(port2_)
{
    setPen(QPen(Qt::NoPen));
    QColor fillColor(Qt::black);
    setBrush(QBrush(fillColor));
    pathStroker.setWidth(3);
    pathStroker.setCapStyle(Qt::RoundCap);
    setZValue(-1);

}

void GraphicsPortConnectionItem::setPos(const QString &port, const QPointF &point)
{
    if (port == port1) {
        point1 = point;
    } else if (port == port2) {
        point2 = point;
    }
    QPainterPath path(point1);
    path.cubicTo(QPointF(point1.x(), 0.5 * (point1.y() + point2.y())), QPointF(point2.x(), 0.5 * (point1.y() + point2.y())), point2);
    setPath(pathStroker.createStroke(path));
}
