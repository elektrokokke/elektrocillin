#ifndef GRAPHICSCLIENTITEM_H
#define GRAPHICSCLIENTITEM_H

#include <QGraphicsRectItem>

class GraphicsClientItem : public QGraphicsRectItem
{
public:
    GraphicsClientItem(const QString &clientName, const QRectF &rect, const QSizeF &innerSize, qreal padding, QGraphicsItem *parent = 0);

    void setInnerItem(QGraphicsItem *item);

    static void fitItemIntoRectangle(QGraphicsItem *item, const QRectF &rect);
};

#endif // GRAPHICSCLIENTITEM_H
