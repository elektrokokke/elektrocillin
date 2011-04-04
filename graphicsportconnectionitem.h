#ifndef GRAPHICSPORTCONNECTIONITEM_H
#define GRAPHICSPORTCONNECTIONITEM_H

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

#include <QMap>
#include <QGraphicsPathItem>
#include <QPainterPathStroker>

class GraphicsPortConnectionItem : public QGraphicsPathItem
{
public:
    GraphicsPortConnectionItem(const QString &port1, const QString &port2, QGraphicsScene *scene);

    void setPos(const QString &port, const QPointF &point);
private:
    QString port1, port2;
    QPointF point1, point2;
    QPainterPathStroker pathStroker;
};

#endif // GRAPHICSPORTCONNECTIONITEM_H
