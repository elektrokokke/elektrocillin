#ifndef GRAPHICSPORTCONNECTIONITEM_H
#define GRAPHICSPORTCONNECTIONITEM_H

#include <QMap>
#include <QGraphicsPathItem>
#include <QPainterPathStroker>

class GraphicsPortConnectionItem : public QGraphicsPathItem
{
public:
    GraphicsPortConnectionItem(const QString &port1, const QString &port2, QGraphicsScene *scene);

    void setPos(const QString &port, const QPointF &point);
private:
    QString port1, port2;
    QPointF point1, point2;
    QPainterPathStroker pathStroker;
};

#endif // GRAPHICSPORTCONNECTIONITEM_H
