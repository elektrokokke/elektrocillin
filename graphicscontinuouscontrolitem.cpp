#include "graphicscontinuouscontrolitem.h"
#include <QtGlobal>
#include <QCursor>
#include <QGraphicsSceneMouseEvent>
#include <QBrush>

GraphicsContinuousControlItem::GraphicsContinuousControlItem(const QString &name_, double minValue_, double maxValue_, double currentValue_, qreal size_, Orientation orientation_, char format_, int precision_, double resolution_, QGraphicsItem *parent) :
    GraphicsLabelItem(parent),
    name(name_),
    minValue(minValue_),
    maxValue(maxValue_),
    currentValue(currentValue_),
    size(size_),
    orientation(orientation_),
    format(format_),
    precision(precision_),
    resolution(resolution_),
    waitingForMouseReleaseEvent(false),
    minLabel(new GraphicsLabelItem(QString("%1: %2").arg(name).arg(minValue, 0, format, precision), parent)),
    maxLabel(new GraphicsLabelItem(QString("%1: %2").arg(name).arg(maxValue, 0, format, precision), parent))
{
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
    setCursor(Qt::ArrowCursor);
    setText(QString("%1: %2").arg(name).arg(currentValue, 0, format, precision));
    minLabel->setBrush(QBrush(Qt::green));
    maxLabel->setBrush(QBrush(Qt::red));
    minLabel->setVisible(false);
    maxLabel->setVisible(false);
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
            // x coordinate has to remain constant:
            newPos.setX(positionBeforeEdit.x());
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
            setText(QString("%1: %2").arg(name).arg(currentValue,  0, 'g', precision));
            valueChanged(currentValue);
        }
        return newPos;
    } else {
        return GraphicsLabelItem::itemChange(change, value);
    }
}

void GraphicsContinuousControlItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    GraphicsLabelItem::mousePressEvent(event);
    if (event->isAccepted()) {
        valueEditingStarted();
        positionBeforeEdit = pos();
        waitingForMouseReleaseEvent = true;
        valueBeforeEdit = currentValue;
        // create label items marking the minimum and maximum value:
        setZValue(zValue() + 1);
        double absolutePositionBeforeEdit = (orientation == HORIZONTAL ? positionBeforeEdit.x() : -positionBeforeEdit.y());
        double relativePositionBeforeEdit = size * (valueBeforeEdit - minValue) / (maxValue - minValue);
        double minPos = absolutePositionBeforeEdit - relativePositionBeforeEdit;
        double maxPos = minPos + size;
        minLabel->setVisible(true);
        maxLabel->setVisible(true);
        if (orientation == HORIZONTAL) {
            minLabel->setPos(QPointF(minPos, positionBeforeEdit.y()));
            maxLabel->setPos(QPointF(maxPos, positionBeforeEdit.y()));
        } else {
            minLabel->setPos(QPointF(positionBeforeEdit.x(), -minPos));
            maxLabel->setPos(QPointF(positionBeforeEdit.x(), -maxPos));
        }
        setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
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
        setZValue(zValue() - 1);
        minLabel->setVisible(false);
        maxLabel->setVisible(false);
        // change our position back to the original position:
        setPos(positionBeforeEdit);
        valueEditingStopped();
    }
}
