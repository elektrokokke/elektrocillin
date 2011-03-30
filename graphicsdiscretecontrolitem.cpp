#include "graphicsdiscretecontrolitem.h"
#include <QtGlobal>

GraphicsDiscreteControlItem::GraphicsDiscreteControlItem(const QString &name, int minValue, int maxValue, int currentValue_, qreal size, GraphicsContinuousControlItem::Orientation orientation, QGraphicsItem *parent) :
    GraphicsContinuousControlItem(name, minValue, maxValue, currentValue_, size, orientation, 'g', -1, 1, parent),
    currentValue(currentValue_)
{
}

void GraphicsDiscreteControlItem::onValueChanged(double value)
{
    int newValue = qRound(value);
    if (newValue != currentValue) {
        currentValue = newValue;
        valueChanged(currentValue);
    }
}
