#include "graphicsclientnameitem.h"

GraphicsClientNameItem::GraphicsClientNameItem(const QString &clientName, QGraphicsItem *parent) :
    QGraphicsRectItem(parent),
    fill(QColor("royalblue").lighter()),
    outline(QBrush(Qt::black), 1),
    font("Helvetica", 16)
{
    setBrush(fill);
    setPen(QPen(Qt::NoPen));

    nameItem = new QGraphicsSimpleTextItem(clientName, this);
    nameItem->setFont(font);
    setRect(nameItem->boundingRect().adjusted(0, 0, 8, 8));
    nameItem->setPos(4, 4);
}

void GraphicsClientNameItem::setMinimumWidth(qreal width)
{
    QRectF rectangle = rect();
    if (rectangle.width() < width) {
        rectangle.setWidth(width);
        setRect(rectangle);
    }
}
