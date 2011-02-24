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
    // set the path according to the given spline:
    double x = interpolator->getX()[0];
    double y = interpolator->evaluate(x);
    if (!bounds.isNull()) {
        y = std::max(std::min(y, bounds.top()), bounds.bottom());
    }
    int previousIndex = 0;
    QPainterPath path(QPointF(x * xscale, y * yscale));
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
    }
    x = interpolator->getX().back();
    y = interpolator->evaluate(x);
    if (!bounds.isNull()) {
        y = std::max(std::min(y, bounds.top()), bounds.bottom());
    }
    path.lineTo(x * xscale, y * yscale);
    setPath(path);
}
