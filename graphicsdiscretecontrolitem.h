#ifndef GRAPHICSDISCRETECONTROLITEM_H
#define GRAPHICSDISCRETECONTROLITEM_H

/*
    Copyright 2011 Arne Jacobs <jarne@jarne.de>

    This file is part of elektrocillin.

    Elektrocillin is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Elektrocillin is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Elektrocillin.  If not, see <http://www.gnu.org/licenses/>.
 */

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
    GraphicsDiscreteControlItem(const QString &name, int minValue, int maxValue, int currentValue, qreal size, GraphicsContinuousControlItem::Orientation orientation = GraphicsContinuousControlItem::HORIZONTAL, QGraphicsItem *parent = 0);
signals:
    void valueChanged(int value);
private slots:
    void onValueChanged(double value);
private:
    int currentValue;
};

#endif // GRAPHICSDISCRETECONTROLITEM_H
