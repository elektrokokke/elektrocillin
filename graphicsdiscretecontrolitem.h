#ifndef GRAPHICSDISCRETECONTROLITEM_H
#define GRAPHICSDISCRETECONTROLITEM_H

#include "graphicscontinuouscontrolitem.h"

/**
  This class uses the continous control item class with appropriate
  parameters to allow only integer values of its value.

  It provides an appropriate signal which signals an integer change.
  */
class GraphicsDiscreteControlItem : public GraphicsContinuousControlItem
{
    Q_OBJECT
public:
    GraphicsDiscreteControlItem(const QString &name, int minValue, int maxValue, int currentValue, qreal size, GraphicsContinuousControlItem::Orientation orientation = GraphicsContinuousControlItem::VERTICAL, QGraphicsItem *parent = 0);
signals:
    void valueChanged(int value);
private slots:
    void onValueChanged(double value);
private:
    int currentValue;
};

#endif // GRAPHICSDISCRETECONTROLITEM_H
