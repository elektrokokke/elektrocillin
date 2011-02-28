#include "graphicsportconnectionitem.h"
#include <QBrush>
#include <QPen>

GraphicsPortConnectionItem::GraphicsPortConnectionItem(const QString &port1_, const QString &port2_, QGraphicsScene *scene) :
    QGraphicsPathItem(0, scene),
    port1(port1_),
    port2(port2_)
{
    setPen(QPen(Qt::NoPen));
    QColor fillColor(Qt::black);
    setBrush(QBrush(fillColor));
    pathStroker.setWidth(3);
    pathStroker.setCapStyle(Qt::RoundCap);
    setZValue(-1);

}

GraphicsPortConnectionItem * GraphicsPortConnectionItem::getPortConnectionItem(const QString &port1, const QString &port2, QGraphicsScene *scene)
{
    GraphicsPortConnectionItem *item = items.value(port1).value(port2, 0);
    if (!item) {
        item = new GraphicsPortConnectionItem(port1, port2, scene);
        items[port1][port2] = item;
        items[port2][port1] = item;
    }
    return item;
}

void GraphicsPortConnectionItem::deletePortConnectionItem(const QString &port1, const QString &port2)
{
    GraphicsPortConnectionItem *item = items.value(port1).value(port2, 0);
    if (item) {
        items[port1].remove(port2);
        items[port2].remove(port1);
        delete item;
    }
}

void GraphicsPortConnectionItem::setPos(const QString &port, const QPointF &point)
{
    if (port == port1) {
        point1 = point;
    } else if (port == port2) {
        point2 = point;
    }
    QPainterPath path(point1);
    path.cubicTo(QPointF(point1.x(), 0.5 * (point1.y() + point2.y())), QPointF(point2.x(), 0.5 * (point1.y() + point2.y())), point2);
    setPath(pathStroker.createStroke(path));
}

void GraphicsPortConnectionItem::setPositions(const QString &port, const QPointF &point)
{
    const QMap<QString, GraphicsPortConnectionItem*> &portItems = items[port];
    for (QMap<QString, GraphicsPortConnectionItem*>::const_iterator i = portItems.begin(); i != portItems.end(); i++) {
        i.value()->setPos(port, point);
    }
}

QMap<QString, QMap<QString, GraphicsPortConnectionItem*> > GraphicsPortConnectionItem::items;
