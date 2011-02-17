#ifndef GRAPHICSPORTITEM_H
#define GRAPHICSPORTITEM_H

#include <QGraphicsRectItem>
#include <QGraphicsSimpleTextItem>

class GraphicsPortItem : public QGraphicsRectItem
{
public:
    GraphicsPortItem(const QString &fullPortName, QGraphicsItem *parent = 0);
private:
    QString fullPortName, shortPortName;
    QGraphicsSimpleTextItem *portNameItem;
};

#endif // GRAPHICSPORTITEM_H
