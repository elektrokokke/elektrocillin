#include "graphicsinterpolationitem.h"
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

void GraphicsInterpolationItem::updatePath()
{
    // set the path according to the given spline:
    QPainterPath path(QPointF(interpolator->getX()[0] * xscale, interpolator->getY()[0] * yscale));
    for (double x = interpolator->getX()[0] + samplingInterval; x < interpolator->getX().back(); x += samplingInterval) {
        double y = interpolator->evaluate(x);
        path.lineTo(x * xscale, y * yscale);
    }
    path.lineTo(interpolator->getX().back() * xscale, interpolator->getY().back() * yscale);
    setPath(path);
}
