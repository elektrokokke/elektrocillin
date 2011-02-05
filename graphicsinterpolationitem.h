#ifndef GRAPHICSCUBICSPLINEITEM_H
#define GRAPHICSCUBICSPLINEITEM_H

#include "interpolator.h"
#include <QGraphicsPathItem>

class GraphicsInterpolationItem : public QGraphicsPathItem
{
public:
    explicit GraphicsInterpolationItem(Interpolator *interpolator, double samplingInterval, QGraphicsItem *parent = 0);

private:
    Interpolator *interpolator;
};

#endif // GRAPHICSCUBICSPLINEITEM_H
