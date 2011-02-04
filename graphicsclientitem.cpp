#include "graphicsclientitem.h"
#include <QGraphicsPathItem>
#include <QGraphicsSimpleTextItem>
#include <QPen>

GraphicsClientItem::GraphicsClientItem(const QString &clientName, const QRectF &rect, const QSizeF &innerSize, qreal padding, QGraphicsItem *parent) :
    QGraphicsRectItem(parent)
{
    setPen(QPen(Qt::NoPen));

    QRectF innerRect = rect.adjusted((rect.width() - innerSize.width()) * 0.5, (rect.height() - innerSize.height()) * 0.5, -(rect.width() - innerSize.width()) * 0.5, -(rect.height() - innerSize.height()) * 0.5);
    setRect(innerRect);

    QPainterPath path(rect.topLeft());
    path.lineTo(rect.bottomLeft());
    path.lineTo(2 * innerRect.left() - rect.left(), rect.bottom());
    path.lineTo(2 * innerRect.left() - rect.left(), innerRect.bottom() + padding);
    path.lineTo(innerRect.left() - padding, innerRect.bottom() + padding);
    path.lineTo(innerRect.left() - padding, innerRect.top() - padding);
    path.lineTo(2 * innerRect.left() - rect.left(), innerRect.top() - padding);
    path.lineTo(2 * innerRect.left() - rect.left(), rect.top());
    path.lineTo(rect.topLeft());
    QGraphicsPathItem *leftBracket = new QGraphicsPathItem(path, this);
    leftBracket->setPen(QPen(Qt::NoPen));
    leftBracket->setBrush(QBrush(Qt::gray));
    QGraphicsPathItem *rightBracket = new QGraphicsPathItem(path, this);
    rightBracket->setTransformOriginPoint(rect.topLeft() * 0.5 + rect.bottomRight() * 0.5);
    rightBracket->setRotation(180);
    rightBracket->setPen(QPen(Qt::NoPen));
    rightBracket->setBrush(QBrush(Qt::gray));

    // create a text showing the client name:
    QGraphicsSimpleTextItem *textItem = new QGraphicsSimpleTextItem(clientName, this);
    fitItemIntoRectangle(textItem, QRectF(innerRect.left(), rect.top() - innerRect.height() * 0.2, innerRect.width(), innerRect.height() * 0.2));
}

void GraphicsClientItem::setInnerItem(QGraphicsItem *item)
{
    // add the item to our children:
    item->setParentItem(this);
    fitItemIntoRectangle(item, rect());
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
