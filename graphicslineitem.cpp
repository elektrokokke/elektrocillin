#include "graphicslineitem.h"

GraphicsLineItem::GraphicsLineItem(QGraphicsItem *parent) :
    QGraphicsLineItem(parent)
{
}

GraphicsLineItem::GraphicsLineItem(const QLineF & line, QGraphicsItem * parent) :
    QGraphicsLineItem(line, parent)
{
}

GraphicsLineItem::GraphicsLineItem(qreal x1, qreal y1, qreal x2, qreal y2, QGraphicsItem * parent) :
    QGraphicsLineItem(x1, y1, x2, y2, parent)
{
}

void GraphicsLineItem::setP1(QPointF point)
{
    QLineF newLine = line();
    newLine.setP1(point);
    setLine(newLine);
    update();
}

void GraphicsLineItem::setP2(QPointF point)
{
    QLineF newLine = line();
    newLine.setP2(point);
    setLine(newLine);
    update();
}
