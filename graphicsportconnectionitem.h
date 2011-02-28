#ifndef GRAPHICSPORTCONNECTIONITEM_H
#define GRAPHICSPORTCONNECTIONITEM_H

#include <QMap>
#include <QGraphicsPathItem>
#include <QPainterPathStroker>

class GraphicsPortConnectionItem : public QGraphicsPathItem
{
private:
    GraphicsPortConnectionItem(const QString &port1, const QString &port2, QGraphicsScene *scene);
public:
    static GraphicsPortConnectionItem * getPortConnectionItem(const QString &port1, const QString &port2, QGraphicsScene *scene);
    static void deletePortConnectionItem(const QString &port1, const QString &port2);
    void setPos(const QString &port, const QPointF &point);
    static void setPositions(const QString &port,const  QPointF &point);
private:
    QString port1, port2;
    QPointF point1, point2;
    QPainterPathStroker pathStroker;
    static QMap<QString, QMap<QString, GraphicsPortConnectionItem*> > items;
};

#endif // GRAPHICSPORTCONNECTIONITEM_H
