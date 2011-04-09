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

#include "graphicsloglineitem.h"

#include <cmath>

GraphicsLogLineItem::GraphicsLogLineItem(const QPointF &p1_, const QPointF &p2_, bool logarithmic_, QGraphicsItem * parent) :
    QGraphicsPathItem(parent),
    p1(p1_),
    p2(p2_),
    logarithmic(logarithmic_)
{
    setPath(constructPath());
}

GraphicsLogLineItem::GraphicsLogLineItem(qreal x1, qreal y1, qreal x2, qreal y2, bool logarithmic_, QGraphicsItem * parent) :
    QGraphicsPathItem(parent),
    p1(QPointF(x1, y1)),
    p2(QPointF(x2, y2)),
    logarithmic(logarithmic_)
{
    setPath(constructPath());
}

void GraphicsLogLineItem::setP1(QPointF point)
{
    p1 = point;
    setPath(constructPath());
}

void GraphicsLogLineItem::setP2(QPointF point)
{
    p2 = point;
    setPath(constructPath());
}

void GraphicsLogLineItem::setLogarithmic(bool logarithmic_)
{
    logarithmic = logarithmic_;
    setPath(constructPath());
}

QPainterPath GraphicsLogLineItem::constructPath()
{
    if (logarithmic) {
        const double base = 1.2;
        QPointF p1_ = p1;
        QPointF p2_ = p2;
        if (p2_.y() < p1_.y()) {
            QPointF swap = p1_;
            p1_ = p2_;
            p2_ = swap;
        }
        QPainterPath path(p1_);
        double yoffset = p2_.y() - p1_.y() + 1.0;
        double xoffset = p2_.x() - p1_.x();
        double logarithm = log(yoffset) / log(base);
        for (double d = 1.0; d < logarithm; d++) {
            yoffset /= base;
            QPointF nextPoint(p1_.x() + xoffset * d / logarithm, p2_.y() + 1.0 - yoffset);
            path.lineTo(nextPoint);
        }
        path.lineTo(p2_);
        return path;
    } else {
        QPainterPath path(p1);
        path.lineTo(p2);
        return path;
    }
}
