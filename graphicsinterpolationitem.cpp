/*
    Copyright 2011 Arne Jacobs

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

#include "graphicsinterpolationitem.h"
#include "linearinterpolator.h"
#include <QPainterPath>

GraphicsInterpolationItem::GraphicsInterpolationItem(Interpolator *interpolator_, double samplingInterval_, QGraphicsItem *parent) :
    QGraphicsPathItem(parent),
    interpolator(interpolator_),
    samplingInterval(samplingInterval_),
    xscale(1),
    yscale(1)
{
    updatePath();
}

GraphicsInterpolationItem::GraphicsInterpolationItem(Interpolator *interpolator_, double samplingInterval_, double xscale_, double yscale_, QGraphicsItem *parent) :
    QGraphicsPathItem(parent),
    interpolator(interpolator_),
    samplingInterval(samplingInterval_),
    xscale(xscale_),
    yscale(yscale_)
{
    updatePath();
}

GraphicsInterpolationItem::GraphicsInterpolationItem(Interpolator *interpolator_, double samplingInterval_, double ymin, double ymax, double xscale_, double yscale_, QGraphicsItem *parent) :
    QGraphicsPathItem(parent),
    interpolator(interpolator_),
    samplingInterval(samplingInterval_),
    xscale(xscale_),
    yscale(yscale_),
    bounds(QRectF(QPointF(interpolator->getX()[0], ymax), QPointF(interpolator->getX().back(), ymin)))
{
    updatePath();
}

Interpolator * GraphicsInterpolationItem::getInterpolator()
{
    return interpolator;
}

void GraphicsInterpolationItem::updatePath()
{
    QPainterPath path;
    if (LinearInterpolator *linearInterpolator = dynamic_cast<LinearInterpolator*>(interpolator)) {
        for (int i = 0; i < linearInterpolator->getX().size(); i++) {
            double x = linearInterpolator->getX()[i];
            double y = linearInterpolator->getY()[i];
            if (i == 0) {
                path.moveTo(x * xscale, y * yscale);
            } else {
                path.lineTo(x * xscale, y * yscale);
            }
        }
    } else {
        // we are traversing the interpolator from start to end, reset it to make this most efficient:
        interpolator->reset();
        // set the path according to the given interpolator:
        double x = interpolator->getX()[0];
        double y = interpolator->evaluate(x);
        if (!bounds.isNull()) {
            y = std::max(std::min(y, bounds.top()), bounds.bottom());
        }
        int previousIndex = 0;
        path.moveTo(QPointF(x * xscale, y * yscale));
        for (; x < interpolator->getX().back(); ) {
            x += samplingInterval;
            if (x > interpolator->getX().back()) {
                x = interpolator->getX().back();
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
                    double yIndex = interpolator->interpolate(previousIndex, interpolator->getX()[previousIndex]);
                    if (!bounds.isNull()) {
                        yIndex = std::max(std::min(yIndex, bounds.top()), bounds.bottom());
                    }
                    path.lineTo(interpolator->getX()[previousIndex] * xscale, yIndex * yscale);
                }
            }
            path.lineTo(x * xscale, y * yscale);
            if (x == interpolator->getX().back()) {
                for (int i = index + 1; i < interpolator->getX().size(); i++) {
                    double yIndex = interpolator->interpolate(i, x);
                    if (!bounds.isNull()) {
                        yIndex = std::max(std::min(yIndex, bounds.top()), bounds.bottom());
                    }
                    path.lineTo(x * xscale, yIndex * yscale);
                }
            }
        }
    }
    setPath(path);
}
