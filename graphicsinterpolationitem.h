#ifndef GRAPHICSCUBICSPLINEITEM_H
#define GRAPHICSCUBICSPLINEITEM_H

#include "interpolator.h"
#include <QGraphicsPathItem>

class GraphicsInterpolationItem : public QGraphicsPathItem
{
public:
    GraphicsInterpolationItem(Interpolator *interpolator, double samplingInterval, QGraphicsItem *parent = 0);
    GraphicsInterpolationItem(Interpolator *interpolator, double samplingInterval, double xscale, double yscale, QGraphicsItem *parent = 0);
    GraphicsInterpolationItem(Interpolator *interpolator, double samplingInterval, double ymin, double ymax, double xscale, double yscale, QGraphicsItem *parent = 0);

    void updatePath();

private:
    Interpolator *interpolator;
    double samplingInterval, xscale, yscale;
    QRectF bounds;
};

#endif // GRAPHICSCUBICSPLINEITEM_H
