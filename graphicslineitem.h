#ifndef GRAPHICSLINEITEM_H
#define GRAPHICSLINEITEM_H

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

#include <QObject>
#include <QGraphicsLineItem>

class GraphicsLineItem : public QObject, public QGraphicsLineItem
{
    Q_OBJECT
public:
    explicit GraphicsLineItem(QGraphicsItem *parent = 0);
    explicit GraphicsLineItem(const QLineF & line, QGraphicsItem * parent = 0);
    explicit GraphicsLineItem(qreal x1, qreal y1, qreal x2, qreal y2, QGraphicsItem * parent = 0);

    enum EndPoints {
        P1,
        P2
    };

public slots:
    void setP1(QPointF point);
    void setP2(QPointF point);

};

#endif // GRAPHICSLINEITEM_H
