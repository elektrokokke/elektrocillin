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

#include "graphicslineitem.h"

GraphicsLineItem::GraphicsLineItem(QGraphicsItem *parent) :
    QGraphicsLineItem(parent)
{
}

GraphicsLineItem::GraphicsLineItem(const QLineF & line, QGraphicsItem * parent) :
    QGraphicsLineItem(line, parent)
{
}

GraphicsLineItem::GraphicsLineItem(qreal x1, qreal y1, qreal x2, qreal y2, QGraphicsItem * parent) :
    QGraphicsLineItem(x1, y1, x2, y2, parent)
{
}

void GraphicsLineItem::setP1(QPointF point)
{
    QLineF newLine = line();
    newLine.setP1(point);
    setLine(newLine);
    update();
}

void GraphicsLineItem::setP2(QPointF point)
{
    QLineF newLine = line();
    newLine.setP2(point);
    setLine(newLine);
    update();
}
