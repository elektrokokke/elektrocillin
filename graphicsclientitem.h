#ifndef GRAPHICSCLIENTITEM_H
#define GRAPHICSCLIENTITEM_H

#include <QGraphicsEllipseItem>
#include <QTransform>

class GraphicsClientItem : public QGraphicsEllipseItem
{
public:
    GraphicsClientItem(const QString &clientName, const QRectF &rect, QGraphicsItem *parent = 0);

    void setInnerItem(QGraphicsItem *item);

    static void fitItemIntoRectangle(QGraphicsItem *item, const QRectF &rect);
};

#endif // GRAPHICSCLIENTITEM_H
