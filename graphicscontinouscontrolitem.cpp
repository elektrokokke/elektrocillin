#include "graphicscontinouscontrolitem.h"
#include <QtGlobal>
#include <QCursor>
#include <QGraphicsSceneMouseEvent>

GraphicsContinousControlItem::GraphicsContinousControlItem(const QString &name_, double minValue_, double maxValue_, double currentValue_, qreal size_, Orientation orientation_, int precision_, double resolution_, QGraphicsItem *parent) :
    GraphicsLabelItem(parent),
    name(name_),
    minValue(minValue_),
    maxValue(maxValue_),
    currentValue(currentValue_),
    size(size_),
    orientation(orientation_),
    precision(precision_),
    resolution(resolution_),
    waitingForMouseReleaseEvent(false)
{
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
    setCursor(Qt::ArrowCursor);
    setText(QString("%1: %2").arg(name).arg(currentValue,  0, 'g', precision));
}

QVariant GraphicsContinousControlItem::itemChange(GraphicsItemChange change, const QVariant & value)
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

void GraphicsContinousControlItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    GraphicsLabelItem::mousePressEvent(event);
    if (event->isAccepted()) {
        valueEditingStarted();
        positionBeforeEdit = pos();
        waitingForMouseReleaseEvent = true;
        valueBeforeEdit = currentValue;
        setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    }
}

void GraphicsContinousControlItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
    if (waitingForMouseReleaseEvent) {
        waitingForMouseReleaseEvent = false;
        setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
        setSelected(false);
        // change our position back to the original position:
        setPos(positionBeforeEdit);
        valueEditingStopped();
    }
}
