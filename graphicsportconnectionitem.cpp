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
