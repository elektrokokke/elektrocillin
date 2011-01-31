#include "graphicsnodeitem.h"
#include <cmath>
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

void GraphicsNodeItem::setSendPositionChanges(bool send)
{
    sendPositionChanges = send;
}

bool GraphicsNodeItem::getSendPositionChanges() const
{
    return sendPositionChanges;
}

bool GraphicsNodeItem::getSendPositionChangesScaled() const
{
    return considerBounds && considerBoundsScaled;
}

void GraphicsNodeItem::setBounds(const QRectF &bounds_)
{
    considerBounds = true;
    bounds = bounds_;
    // adjust the coordinates to be within the specified bounds:
    setPos(adjustToBounds(pos()));
}

void GraphicsNodeItem::setBoundsScaled(const QRectF &boundsScaled_)
{
    considerBoundsScaled = true;
    boundsScaled = boundsScaled_;
}

void GraphicsNodeItem::setScale(GraphicsNodeItem::Scale horizontal, GraphicsNodeItem::Scale vertical)
{
    horizontalScale = horizontal;
    verticalScale = vertical;
}

void GraphicsNodeItem::resetBounds()
{
    bounds = QRectF();
    considerBounds = false;
}

void GraphicsNodeItem::resetBoundsScaled()
{
    boundsScaled = QRectF();
    considerBoundsScaled = false;
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
    if ((change == ItemPositionChange) && !changingCoordinates && (getSendPositionChanges() || getSendPositionChangesScaled())) {
        changingCoordinates = true;
        QPointF newPosition = value.toPointF();
        // adjust the coordinates to be within the specified bounds:
        newPosition = adjustToBounds(newPosition);
        if (newPosition != pos()) {
            if (getSendPositionChanges()) {
                positionChanged(newPosition);
                if (newPosition.x() != pos().x()) {
                    xChanged(newPosition.x());
                }
                if (newPosition.y() != pos().y()) {
                    yChanged(newPosition.y());
                }
            }
            if (getSendPositionChangesScaled()) {
                // convert to scaled coordinates:
                QPointF newPositionScaled = scale(newPosition);
                positionChangedScaled(newPositionScaled);
                if (newPosition.x() != pos().x()) {
                    xChangedScaled(newPositionScaled.x());
                }
                if (newPosition.y() != pos().y()) {
                    yChangedScaled(newPositionScaled.y());
                }
            }
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
    considerBoundsScaled = false;
    changingCoordinates = false;
    sendPositionChanges = false;
    horizontalScale = LOGARITHMIC;
    verticalScale = LINEAR;
    setAcceptHoverEvents(true);
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemSendsGeometryChanges);// | QGraphicsItem::ItemIsSelectable);
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

QPointF GraphicsNodeItem::scale(const QPointF &p)
{
    if (getSendPositionChangesScaled()) {
        QPointF scaled;
        if (horizontalScale == LINEAR) {
            scaled.setX((p.x() - bounds.left()) / (bounds.right() - bounds.left()) * (boundsScaled.right() - boundsScaled.left()) + boundsScaled.left());
        } else {
            scaled.setX(exp((p.x() - bounds.left()) / (bounds.right() - bounds.left()) * log(boundsScaled.right() / boundsScaled.left())) * boundsScaled.left());
        }
        if (verticalScale == LINEAR) {
            scaled.setY((p.y() - bounds.top()) / (bounds.bottom() - bounds.top()) * (boundsScaled.bottom() - boundsScaled.top()) + boundsScaled.top());
        } else {
            scaled.setY(exp((p.y() - bounds.top()) / (bounds.bottom() - bounds.top()) * log(boundsScaled.bottom() / boundsScaled.top())) * boundsScaled.top());
        }
        return scaled;
    } else {
        return p;
    }
}
