#ifndef GRAPHICSCLIENTITEM2_H
#define GRAPHICSCLIENTITEM2_H

#include <QGraphicsPathItem>

class GraphicsClientItem2 : public QGraphicsPathItem
{
public:
    GraphicsClientItem2(const QString &clientName, QGraphicsItem *parent = 0);
};

#endif // GRAPHICSCLIENTITEM2_H
