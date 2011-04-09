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

#include "graphicsinterpolationitem.h"
#include "linearinterpolator.h"
#include <QPainterPath>

GraphicsInterpolationItem::GraphicsInterpolationItem(AbstractInterpolator *interpolator_, double samplingInterval_, QGraphicsItem *parent) :
    QGraphicsPathItem(parent),
    interpolator(interpolator_),
    samplingInterval(samplingInterval_),
    xscale(1),
    yscale(1)
{
    updatePath();
}

GraphicsInterpolationItem::GraphicsInterpolationItem(AbstractInterpolator *interpolator_, double samplingInterval_, double xscale_, double yscale_, QGraphicsItem *parent) :
    QGraphicsPathItem(parent),
    interpolator(interpolator_),
    samplingInterval(samplingInterval_),
    xscale(xscale_),
    yscale(yscale_)
{
    updatePath();
}

GraphicsInterpolationItem::GraphicsInterpolationItem(AbstractInterpolator *interpolator_, double samplingInterval_, double ymin, double ymax, double xscale_, double yscale_, QGraphicsItem *parent) :
    QGraphicsPathItem(parent),
    interpolator(interpolator_),
    samplingInterval(samplingInterval_),
    xscale(xscale_),
    yscale(yscale_),
    bounds(QRectF(QPointF(interpolator->getControlPoint(0).x(), ymax), QPointF(interpolator->getControlPoint(interpolator->getNrOfControlPoints() - 1).x(), ymin)))
{
    updatePath();
}

AbstractInterpolator * GraphicsInterpolationItem::getInterpolator()
{
    return interpolator;
}

void GraphicsInterpolationItem::updatePath()
{
    // set the path according to the given interpolator:
    int nrOfControlPoints = interpolator->getNrOfControlPoints();
    QPainterPath path;
    QPointF firstControlPoint = interpolator->getControlPoint(0);
    QPointF lastControlPoint = interpolator->getControlPoint(nrOfControlPoints - 1);
    double x = firstControlPoint.x();
    double y = firstControlPoint.y();
    int previousIndex = 0;
    path.moveTo(QPointF(x * xscale, y * yscale));
    for (; x < lastControlPoint.x(); ) {
        x += samplingInterval;
        if (x > lastControlPoint.x()) {
            x = lastControlPoint.x();
        }
        int index;
        y = interpolator->evaluate(x, &index);
        if (!bounds.isNull()) {
            y = std::max(std::min(y, bounds.top()), bounds.bottom());
        }
        // make sure to not miss a control point:
        if (index != previousIndex) {
            for (; previousIndex < index; ) {
                previousIndex++;
                QPointF controlPoint = interpolator->getControlPoint(previousIndex);
                double xIndex = controlPoint.x();
                double yIndex = controlPoint.y();
                if (!bounds.isNull()) {
                    yIndex = std::max(std::min(yIndex, bounds.top()), bounds.bottom());
                }
                path.lineTo(xIndex * xscale, yIndex * yscale);
            }
        }
        path.lineTo(x * xscale, y * yscale);
        if (x == lastControlPoint.x()) {
            for (int i = index + 1; i < nrOfControlPoints; i++) {
                QPointF controlPoint = interpolator->getControlPoint(i);
                double xIndex = controlPoint.x();
                double yIndex = controlPoint.y();
                if (!bounds.isNull()) {
                    yIndex = std::max(std::min(yIndex, bounds.top()), bounds.bottom());
                }
                path.lineTo(xIndex * xscale, yIndex * yscale);
            }
        }
    }
    setPath(path);
}
