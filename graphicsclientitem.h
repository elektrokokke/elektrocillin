#ifndef GRAPHICSCLIENTITEM_H
#define GRAPHICSCLIENTITEM_H

#include "jackclient.h"
#include <QGraphicsRectItem>
#include <QTransform>
#include <QBrush>
#include <QPen>

class GraphicsClientItem : public QGraphicsRectItem
{
public:
    GraphicsClientItem(JackClient *client, QGraphicsItem *parent = 0);
    GraphicsClientItem(JackClient *client, const QString &clientName, QGraphicsItem *parent = 0);

    JackClient * getClient();

    const QRectF & getInnerRect() const;

    QGraphicsItem * getInnerItem();
    void setInnerItem(QGraphicsItem *item);

    static void fitItemIntoRectangle(QGraphicsItem *item, const QRectF &rect);

private:
    JackClient *client;
    QString clientName;
    QRectF innerRect;
    QGraphicsItem *innerItem;
    int padding;
    QBrush fill;
    QPen outline;

    void init();
};

#endif // GRAPHICSCLIENTITEM_H
