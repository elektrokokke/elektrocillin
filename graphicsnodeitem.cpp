#include "graphicsnodeitem.h"

#include <QPen>

GraphicsNodeItem::GraphicsNodeItem(QGraphicsItem *parent) :
    QGraphicsEllipseItem(parent)
{
    init();
}

GraphicsNodeItem::GraphicsNodeItem(const QRectF & rect, QGraphicsItem * parent) :
    QGraphicsEllipseItem(rect, parent)
{
    init();
}

GraphicsNodeItem::GraphicsNodeItem(qreal x, qreal y, qreal width, qreal height, QGraphicsItem * parent) :
    QGraphicsEllipseItem(x, y, width, height, parent)
{
    init();
}

void GraphicsNodeItem::setBounds(const QRectF &bounds_)
{
    considerBounds = true;
    bounds = bounds_;
    // adjust the coordinates to be within the specified bounds:
    setPos(adjustToBounds(pos()));
}

void GraphicsNodeItem::resetBounds()
{
    bounds = QRectF();
    considerBounds = false;
}

void GraphicsNodeItem::setBoundsLeft(qreal left)
{
    bounds.setLeft(left);
    // adjust the coordinates to be within the specified bounds:
    setPos(adjustToBounds(pos()));
}

void GraphicsNodeItem::setBoundsRight(qreal right)
{
    bounds.setRight(right);
    // adjust the coordinates to be within the specified bounds:
    setPos(adjustToBounds(pos()));
}

void GraphicsNodeItem::connectLine(GraphicsLineItem *line, GraphicsLineItem::EndPoints endPoint)
{
    if (endPoint == GraphicsLineItem::P1) {
        line->setP1(pos());
        QObject::connect(this, SIGNAL(positionChanged(QPointF)), line, SLOT(setP1(QPointF)));
    } else {
        line->setP2(pos());
        QObject::connect(this, SIGNAL(positionChanged(QPointF)), line, SLOT(setP2(QPointF)));
    }
}

void GraphicsNodeItem::connectLine(GraphicsLogLineItem *line, GraphicsLineItem::EndPoints endPoint)
{
    if (endPoint == GraphicsLineItem::P1) {
        line->setP1(pos());
        QObject::connect(this, SIGNAL(positionChanged(QPointF)), line, SLOT(setP1(QPointF)));
    } else {
        line->setP2(pos());
        QObject::connect(this, SIGNAL(positionChanged(QPointF)), line, SLOT(setP2(QPointF)));
    }
}

void GraphicsNodeItem::setX(qreal x)
{
    QGraphicsEllipseItem::setX(x);
}

void GraphicsNodeItem::setY(qreal y)
{
    QGraphicsEllipseItem::setY(y);
}

void GraphicsNodeItem::hoverEnterEvent( QGraphicsSceneHoverEvent * event )
{
    QRectF newRect = oldRect = rect();
    newRect.setX(oldRect.x() - 1);
    newRect.setY(oldRect.y() - 1);
    newRect.setWidth(oldRect.width() + 2);
    newRect.setHeight(oldRect.height() + 2);
    setRect(newRect);
    QGraphicsEllipseItem::hoverEnterEvent(event);
}

void GraphicsNodeItem::hoverLeaveEvent( QGraphicsSceneHoverEvent * event )
{
    setRect(oldRect);
    QGraphicsEllipseItem::hoverLeaveEvent(event);
}

QVariant GraphicsNodeItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if ((change == ItemPositionChange) && !changingCoordinates) {
        changingCoordinates = true;
        QPointF newPosition = value.toPointF();
        // adjust the coordinates to be within the specified bounds:
        newPosition = adjustToBounds(newPosition);
        if (newPosition != pos()) {
            positionChanged(newPosition);
            xChanged(newPosition.x());
            yChanged(newPosition.y());
        }
        changingCoordinates = false;
        return QGraphicsItem::itemChange(change, newPosition);
    } else {
        return QGraphicsItem::itemChange(change, value);
    }
}

void GraphicsNodeItem::init()
{
    considerBounds = false;
    changingCoordinates = false;
    setAcceptHoverEvents(true);
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemSendsGeometryChanges | QGraphicsItem::ItemIsSelectable);
}

QPointF GraphicsNodeItem::adjustToBounds(const QPointF &point)
{
    if (considerBounds && !bounds.contains(point)) {
        QPointF adjusted = point;
        if (adjusted.x() < bounds.left()) {
            adjusted.setX(bounds.left());
        }
        if (adjusted.x() > bounds.right()) {
            adjusted.setX(bounds.right());
        }
        if (adjusted.y() < bounds.top()) {
            adjusted.setY(bounds.top());
        }
        if (adjusted.y() > bounds.bottom()) {
            adjusted.setY(bounds.bottom());
        }
        return adjusted;
    } else {
        return point;
    }
}
