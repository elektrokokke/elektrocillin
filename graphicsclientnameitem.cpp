#include "graphicsclientnameitem.h"
#include <QGraphicsDropShadowEffect>

GraphicsClientNameItem::GraphicsClientNameItem(const QString &clientName, QGraphicsItem *parent) :
    QGraphicsRectItem(parent),
    fill(QColor("royalblue").lighter()),
    outline(Qt::black),
    font("Helvetica", 16)
{
    setBrush(fill);
    setPen(outline);

    nameItem = new QGraphicsSimpleTextItem(clientName, this);
    nameItem->setFont(font);
    setRect(nameItem->boundingRect().adjusted(0, 0, 8, 8));
    nameItem->setPos(4, 4);

    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect();
    setGraphicsEffect(effect);
}

void GraphicsClientNameItem::setMinimumWidth(qreal width)
{
    QRectF rectangle = rect();
    if (rectangle.width() < width) {
        rectangle.setWidth(width);
        setRect(rectangle);
    }
}
