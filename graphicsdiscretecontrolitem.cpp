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

#include "graphicsdiscretecontrolitem.h"
#include <QtGlobal>

GraphicsDiscreteControlItem::GraphicsDiscreteControlItem(const QString &name, int minValue, int maxValue, int currentValue_, qreal size, GraphicsContinuousControlItem::Orientation orientation, QGraphicsItem *parent) :
    GraphicsContinuousControlItem(name, minValue, maxValue, currentValue_, size, orientation, 'g', -1, 1, parent),
    currentValue(currentValue_)
{
    QObject::connect(this, SIGNAL(valueChanged(double)), this, SLOT(onValueChanged(double)));
}

void GraphicsDiscreteControlItem::onValueChanged(double value)
{
    int newValue = qRound(value);
    if (newValue != currentValue) {
        currentValue = newValue;
        valueChanged(currentValue);
    }
}
