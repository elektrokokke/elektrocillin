#include "graphicsclientitem.h"
#include <QGraphicsPathItem>
#include <QGraphicsSimpleTextItem>
#include <QPen>
#include <cmath>

GraphicsClientItem::GraphicsClientItem(JackClient *client_, const QRectF &rect, QGraphicsItem *parent) :
    QGraphicsEllipseItem(rect, parent),
    client(client_),
    innerItem(0)
{
    setPen(QPen(QBrush(Qt::gray), 7, Qt::DashLine));
    setBrush(QBrush(QColor(Qt::gray).lighter()));
    QPointF offset = (rect.bottomRight() - rect.topLeft()) * 0.25 * (2 - sqrt(2.0));
    innerRect = QRectF(rect.topLeft() + offset, rect.bottomRight() - offset);
}

JackClient * GraphicsClientItem::getClient()
{
    return client;
}

const QRectF & GraphicsClientItem::getInnerRect() const
{
    return innerRect;
}

QGraphicsItem * GraphicsClientItem::getInnerItem()
{
    return innerItem;
}

void GraphicsClientItem::setInnerItem(QGraphicsItem *item)
{
    innerItem = item;
    // add the item to our children:
    item->setParentItem(this);
    fitItemIntoRectangle(item, getInnerRect());
}

void GraphicsClientItem::fitItemIntoRectangle(QGraphicsItem *item, const QRectF &rect)
{
    QRectF boundingRect = item->boundingRect();
    // scale the item such that it fits into our rectangle:
    qreal scale = qMin(rect.width() / boundingRect.width(), rect.height() / boundingRect.height());
    item->setScale(scale);
    QPointF oldTopLeftScaled(boundingRect.topLeft() * scale);
    QPointF newTopLeftScaled(rect.left() + (rect.width() - boundingRect.width() * scale) * 0.5, rect.top() + (rect.height() - boundingRect.height() * scale) * 0.5);
    item->setPos(item->pos() - oldTopLeftScaled + newTopLeftScaled);
}
