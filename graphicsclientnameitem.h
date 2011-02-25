#ifndef GRAPHICSCLIENTNAMEITEM_H
#define GRAPHICSCLIENTNAMEITEM_H

#include <QGraphicsRectItem>
#include <QBrush>
#include <QPen>
#include <QFont>

class GraphicsClientNameItem : public QGraphicsRectItem
{
public:
    GraphicsClientNameItem(const QString &clientName, QGraphicsItem *parent = 0);

    void setMinimumWidth(qreal width);

private:
    QGraphicsSimpleTextItem *nameItem;
    QBrush fill;
    QPen outline;
    QFont font;
};

#endif // GRAPHICSCLIENTNAMEITEM_H
