#ifndef GRAPHICSCUBICSPLINEITEM_H
#define GRAPHICSCUBICSPLINEITEM_H

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
#include <QGraphicsPathItem>

class GraphicsInterpolationItem : public QGraphicsPathItem
{
public:
    GraphicsInterpolationItem(AbstractInterpolator *interpolator, double samplingInterval, QGraphicsItem *parent = 0);
    GraphicsInterpolationItem(AbstractInterpolator *interpolator, double samplingInterval, double xscale, double yscale, QGraphicsItem *parent = 0);
    GraphicsInterpolationItem(AbstractInterpolator *interpolator, double samplingInterval, double ymin, double ymax, double xscale, double yscale, QGraphicsItem *parent = 0);

    AbstractInterpolator * getInterpolator();

    void updatePath();

private:
    AbstractInterpolator *interpolator;
    double samplingInterval, xscale, yscale;
    QRectF bounds;
};

#endif // GRAPHICSCUBICSPLINEITEM_H
