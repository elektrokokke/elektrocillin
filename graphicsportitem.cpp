#include "graphicsportitem.h"
#include <QBrush>
#include <QFont>

GraphicsPortItem::GraphicsPortItem(const QString &fullPortName_, QGraphicsItem *parent) :
    QGraphicsRectItem(parent),
    fullPortName(fullPortName_),
    shortPortName(fullPortName.split(":")[1]),
    portNameItem(new QGraphicsSimpleTextItem(shortPortName, this))
{
    setBrush(QBrush(Qt::white));
    portNameItem->setFont(QFont("Helvetica", 8));
    setRect(portNameItem->boundingRect().adjusted(0, 0, 4, 2));
    portNameItem->setPos(2, 1);
}
