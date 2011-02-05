#include "graphicsinterpolationitem.h"
#include <QPainterPath>

GraphicsInterpolationItem::GraphicsInterpolationItem(Interpolator *interpolator_, double samplingInterval, QGraphicsItem *parent) :
    QGraphicsPathItem(parent),
    interpolator(interpolator_)
{
    // set the path according to the given spline:
    QPainterPath path(QPointF(interpolator->getX()[0], interpolator->getY()[0]));
    for (double x = interpolator->getX()[0] + samplingInterval; x < interpolator->getX().back(); x += samplingInterval) {
        double y = interpolator->evaluate(x);
        path.lineTo(x, y);
    }
    path.lineTo(interpolator->getX().back(), interpolator->getY().back());
    setPath(path);
}
