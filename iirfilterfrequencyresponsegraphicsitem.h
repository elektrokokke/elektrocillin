#ifndef IIRFILTERFREQUENCYRESPONSEGRAPHICSITEM_H
#define IIRFILTERFREQUENCYRESPONSEGRAPHICSITEM_H

#include "frequencyresponsegraphicsitem.h"
#include "iirfilter.h"

class IIRFilterFrequencyResponseGraphicsItem : public FrequencyResponseGraphicsItem
{
public:
    IIRFilterFrequencyResponseGraphicsItem(IIRFilter *filter, const QRectF &rect, double lowestHertz, double highestHertz, double lowestDecibel = -40, double highestDecibel = 40, QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);

protected:
    virtual double getSquaredAmplitudeResponse(double frequencyInHertz);

private:
    IIRFilter *filter;
};

#endif // IIRFILTERFREQUENCYRESPONSEGRAPHICSITEM_H
