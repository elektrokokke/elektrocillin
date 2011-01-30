#include "iirfilterfrequencyresponsegraphicsitem.h"

IIRFilterFrequencyResponseGraphicsItem::IIRFilterFrequencyResponseGraphicsItem(IIRFilter *filter_, const QRectF &rect, double lowestHertz, double highestHertz, double lowestDecibel, double highestDecibel, QGraphicsItem *parent, QGraphicsScene *scene) :
    FrequencyResponseGraphicsItem(rect, lowestHertz, highestHertz, lowestDecibel, highestDecibel, parent, scene),
    filter(filter_)
{
}

double IIRFilterFrequencyResponseGraphicsItem::getSquaredAmplitudeResponse(double frequencyInHertz)
{
    return filter->getSquaredAmplitudeResponse(frequencyInHertz);
}
