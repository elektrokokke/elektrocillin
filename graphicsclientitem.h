#ifndef GRAPHICSCLIENTITEM_H
#define GRAPHICSCLIENTITEM_H

#include "jackclient.h"
#include <QGraphicsEllipseItem>
#include <QTransform>

class GraphicsClientItem : public QGraphicsEllipseItem
{
public:
    GraphicsClientItem(JackClient *client, const QRectF &rect, QGraphicsItem *parent = 0);

    JackClient * getClient();

    const QRectF & getInnerRect() const;

    QGraphicsItem * getInnerItem();
    void setInnerItem(QGraphicsItem *item);

    static void fitItemIntoRectangle(QGraphicsItem *item, const QRectF &rect);

private:
    JackClient *client;
    QRectF innerRect;
    QGraphicsItem *innerItem;
};

#endif // GRAPHICSCLIENTITEM_H
