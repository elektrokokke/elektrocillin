#ifndef GRAPHICSCUBICSPLINEITEM_H
#define GRAPHICSCUBICSPLINEITEM_H

#include "interpolator.h"
#include <QGraphicsPathItem>

class GraphicsInterpolationItem : public QGraphicsPathItem
{
public:
    explicit GraphicsInterpolationItem(Interpolator *interpolator, double samplingInterval, QGraphicsItem *parent = 0);

    void updatePath();

private:
    Interpolator *interpolator;
    double samplingInterval;
};

#endif // GRAPHICSCUBICSPLINEITEM_H
