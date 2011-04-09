#ifndef GRAPHICSLOGLINEITEM_H
#define GRAPHICSLOGLINEITEM_H

/*
    Copyright 2011 Arne Jacobs <jarne@jarne.de>

    This file is part of elektrocillin.

    Elektrocillin is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Elektrocillin is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Elektrocillin.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QGraphicsPathItem>

class GraphicsLogLineItem : public QObject, public QGraphicsPathItem
{
    Q_OBJECT
public:
    explicit GraphicsLogLineItem(const QPointF &p1, const QPointF &p2, bool logarithmic = false, QGraphicsItem * parent = 0);
    explicit GraphicsLogLineItem(qreal x1, qreal y1, qreal x2, qreal y2, bool logarithmic = false, QGraphicsItem * parent = 0);

public slots:
    void setP1(QPointF point);
    void setP2(QPointF point);
    void setLogarithmic(bool logarithmic);

private:
    QPainterPath constructPath();

    QPointF p1, p2;
    bool logarithmic;
};

#endif // GRAPHICSLOGLINEITEM_H
