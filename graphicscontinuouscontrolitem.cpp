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
#include <QtGlobal>
#include <QCursor>
#include <QGraphicsSceneMouseEvent>
#include <QBrush>
#include <QPen>

GraphicsContinuousControlItem::GraphicsContinuousControlItem(const QString &name_, double minValue_, double maxValue_, double currentValue_, qreal size_, Orientation orientation_, char format_, int precision_, double resolution_, QGraphicsItem *parent) :
    GraphicsLabelItem(parent),
    name(name_),
    minValue(minValue_),
    maxValue(maxValue_),
    currentValue(currentValue_),
    size(size_),
    orientation(orientation_),
    horizontalAlignment(LEFT),
    verticalAlignment(TOP),
    format(format_),
    precision(precision_),
    resolution(resolution_),
    waitingForMouseReleaseEvent(false),
    backgroundRect(new QGraphicsRectItem(parent)),
    minLabel(new GraphicsLabelItem(parent)),
    maxLabel(new GraphicsLabelItem(parent))
{
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
    setCursor(Qt::PointingHandCursor);
    setPen(QPen(QBrush(QColor("steelblue")), 2));
    backgroundRect->setPen(QPen(QBrush(Qt::black), 1));
    backgroundRect->setBrush(minLabel->brush());
    backgroundRect->setVisible(false);
    setLabelText(this, currentValue);
    setLabelText(minLabel, minValue);
    setLabelText(maxLabel, maxValue);
    minLabel->setPen(QPen(Qt::NoPen));
    maxLabel->setPen(QPen(Qt::NoPen));
    minLabel->setBrush(QBrush(Qt::NoBrush));
    maxLabel->setBrush(QBrush(Qt::NoBrush));
    minLabel->setVisible(false);
    maxLabel->setVisible(false);
    backgroundRect->setZValue(1);
    minLabel->setZValue(1);
    maxLabel->setZValue(1);
}

GraphicsContinuousControlItem::GraphicsContinuousControlItem(const ParameterProcessor::Parameter &parameter, qreal size_, Orientation orientation_, char format_, int precision_, QGraphicsItem *parent) :
    GraphicsLabelItem(parent),
    name(parameter.name),
    minValue(parameter.min),
    maxValue(parameter.max),
    currentValue(parameter.value),
    stringValues(parameter.stringValues),
    size(size_),
    orientation(orientation_),
    horizontalAlignment(LEFT),
    verticalAlignment(TOP),
    format(format_),
    precision(precision_),
    resolution(parameter.resolution),
    waitingForMouseReleaseEvent(false),
    backgroundRect(new QGraphicsRectItem(parent)),
    minLabel(new GraphicsLabelItem(parent)),
    maxLabel(new GraphicsLabelItem(parent))
{
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
    setCursor(Qt::PointingHandCursor);
    setPen(QPen(QBrush(QColor("steelblue")), 2));
    backgroundRect->setPen(QPen(QBrush(Qt::black), 1));
    backgroundRect->setBrush(minLabel->brush());
    backgroundRect->setVisible(false);
    setLabelText(this, currentValue);
    setLabelText(minLabel, minValue);
    setLabelText(maxLabel, maxValue);
    minLabel->setPen(QPen(Qt::NoPen));
    maxLabel->setPen(QPen(Qt::NoPen));
    minLabel->setBrush(QBrush(Qt::NoBrush));
    maxLabel->setBrush(QBrush(Qt::NoBrush));
    minLabel->setVisible(false);
    maxLabel->setVisible(false);
    backgroundRect->setZValue(1);
    minLabel->setZValue(1);
    maxLabel->setZValue(1);
}

void GraphicsContinuousControlItem::setHorizontalAlignment(HorizontalAlignment horizontalAlignment)
{
    this->horizontalAlignment = horizontalAlignment;
}

void GraphicsContinuousControlItem::setMinValue(double minValue)
{
    this->minValue = minValue;
    setValue(currentValue, false);
    setLabelText(minLabel, minValue);
}

void GraphicsContinuousControlItem::setMaxValue(double maxValue)
{
    this->maxValue = maxValue;
    setValue(currentValue, false);
    setLabelText(maxLabel, maxValue);
}

void GraphicsContinuousControlItem::setSize(double size)
{
    this->size = size;
}

void GraphicsContinuousControlItem::setValue(double value, bool emitSignal)
{
    if (resolution) {
        value = qBound(minValue, (double)qRound(value / resolution) * resolution, maxValue);
    }
    if (value != currentValue) {
        QRectF previousRect = rect().translated(pos());
        currentValue = value;
        // change the text accordingly:
        setLabelText(this, currentValue);
        if (emitSignal) {
            valueChanged(currentValue);
        }
        // align the new position:
        setPos(alignX(rect(), previousRect), pos().y());
    }
}

QVariant GraphicsContinuousControlItem::itemChange(GraphicsItemChange change, const QVariant & value)
{
    if (change == QGraphicsItem::ItemPositionChange) {
        if (minValue == maxValue) {
            return pos();
        }
        QPointF newPos = value.toPointF();
        double absolutePositionBeforeEdit = (orientation == HORIZONTAL ? positionBeforeEdit.x() : -positionBeforeEdit.y());
        double relativePositionBeforeEdit = size * (valueBeforeEdit - minValue) / (maxValue - minValue);
        // make sure the element is not moved beyond its limits:
        double minPos = absolutePositionBeforeEdit - relativePositionBeforeEdit;
        double maxPos = minPos + size;
        double currentRelativePosition;
        if (orientation == HORIZONTAL) {
            // y coordinate has to remain constant:
            newPos.setY(positionBeforeEdit.y());
            newPos.setX(qBound(minPos, newPos.x(), maxPos));
            // update the relative position:
            currentRelativePosition = newPos.x() - minPos;
        } else {
            newPos.setY(-qBound(minPos, -newPos.y(), maxPos));
            // update the relative position:
            currentRelativePosition = -newPos.y() - minPos;
        }
        // change the current value according to the position change:
        double nextValue = currentRelativePosition * (maxValue - minValue) / size;
        // if given resolution is non-zero, change value such that
        // nextValue is a multiple of the resolution:
        if (resolution) {
            nextValue = (double)qRound(nextValue / resolution) * resolution;
            currentRelativePosition = nextValue * size / (maxValue - minValue);
            if (orientation == HORIZONTAL) {
                newPos.setX(currentRelativePosition + minPos);
            } else {
                newPos.setY(-currentRelativePosition - minPos);
            }
        }
        nextValue += minValue;
        if (nextValue != currentValue) {
            currentValue = nextValue;
            // change the text accordingly:
            setLabelText(this, currentValue);
            valueChanged(currentValue);
        }
        if (orientation == VERTICAL) {
            // align the new position:
            newPos.setX(alignX(rect(), rectBeforeEdit.translated(positionBeforeEdit)));
        }
        return newPos;
    } else {
        return GraphicsLabelItem::itemChange(change, value);
    }
}

void GraphicsContinuousControlItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    if (minValue != maxValue) {
        GraphicsLabelItem::mousePressEvent(event);
        if (event->isAccepted()) {
            valueEditingStarted();
            positionBeforeEdit = pos();
            rectBeforeEdit = rect();
            waitingForMouseReleaseEvent = true;
            valueBeforeEdit = currentValue;
            // create label items marking the minimum and maximum value:
            setZValue(zValue() + 2);
            double absolutePositionBeforeEdit = (orientation == HORIZONTAL ? positionBeforeEdit.x() : -positionBeforeEdit.y());
            double relativePositionBeforeEdit = size * (valueBeforeEdit - minValue) / (maxValue - minValue);
            double minPos = absolutePositionBeforeEdit - relativePositionBeforeEdit;
            double maxPos = minPos + size;
            backgroundRect->setVisible(true);
            minLabel->setVisible(true);
            maxLabel->setVisible(true);
            if (orientation == HORIZONTAL) {
                minLabel->setPos(QPointF(minPos, positionBeforeEdit.y()));
                maxLabel->setPos(QPointF(maxPos, positionBeforeEdit.y()));
            } else {
                minLabel->setPos(QPointF(alignX(minLabel->rect(), rectBeforeEdit.translated(positionBeforeEdit)), -minPos));
                maxLabel->setPos(QPointF(alignX(maxLabel->rect(), rectBeforeEdit.translated(positionBeforeEdit)), -maxPos));
            }
            backgroundRect->setRect((minLabel->rect().translated(minLabel->pos()) | maxLabel->rect().translated(maxLabel->pos())));//.adjusted(-1, -1, 1, 1));
            setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
        }
    }
}

void GraphicsContinuousControlItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
    GraphicsLabelItem::mouseReleaseEvent(event);
    if (waitingForMouseReleaseEvent) {
        waitingForMouseReleaseEvent = false;
        setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
        setSelected(false);
        // remove the minimum/maximum labels:
        setZValue(zValue() - 2);
        backgroundRect->setVisible(false);
        minLabel->setVisible(false);
        maxLabel->setVisible(false);
        // change our position back to the original position:
        setPos(alignX(rect(), rectBeforeEdit.translated(positionBeforeEdit)), positionBeforeEdit.y());
        valueEditingStopped(currentValue);
    }
}

qreal GraphicsContinuousControlItem::alignX(QRectF rect, QRectF rectAlign)
{
    if (horizontalAlignment == LEFT) {
        return rectAlign.x();
    } else {
        return rectAlign.x() + rectAlign.width() - rect.width();
    }
}

void GraphicsContinuousControlItem::setLabelText(GraphicsLabelItem *label, double value)
{
    QString stringValue = stringValues.value(value, QString());
    if (stringValue.isNull()) {
        label->setText(QString("%1: %2").arg(name).arg(value, 0, format, precision));
    } else {
        label->setText(QString("%1: %2").arg(name).arg(stringValue));
    }
}
