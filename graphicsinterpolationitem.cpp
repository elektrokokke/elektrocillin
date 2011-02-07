#include "graphicsinterpolationitem.h"
#include <QPainterPath>
#include <QDebug>

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

void GraphicsInterpolationItem::updatePath()
{
    qDebug() << xscale << yscale << bounds;
    // set the path according to the given spline:
    if (bounds.isNull()) {
        QPainterPath path(QPointF(interpolator->getX()[0] * xscale, interpolator->getY()[0] * yscale));
        for (double x = interpolator->getX()[0] + samplingInterval; x < interpolator->getX().back(); x += samplingInterval) {
            double y = interpolator->evaluate(x);
            path.lineTo(x * xscale, y * yscale);
        }
        path.lineTo(interpolator->getX().back() * xscale, interpolator->getY().back() * yscale);
        setPath(path);
    } else {
        double x = interpolator->getX()[0];
        double xEnd = interpolator->getX().back();
        double y = std::max(std::min(interpolator->evaluate(x), bounds.top()), bounds.bottom());
        QPainterPath path(QPointF(x * xscale, y * yscale));
        for (; x < xEnd; x += samplingInterval) {
            y = std::max(std::min(interpolator->evaluate(x), bounds.top()), bounds.bottom());
            path.lineTo(x * xscale, y * yscale);
        }
        y = std::max(std::min(interpolator->evaluate(xEnd), bounds.top()), bounds.bottom());
        path.lineTo(xEnd * xscale, y * yscale);
        setPath(path);
    }
}
