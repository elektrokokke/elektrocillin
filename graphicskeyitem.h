#ifndef GRAPHICSKEYITEM_H
#define GRAPHICSKEYITEM_H

#include <QGraphicsPolygonItem>
#include <QBrush>
#include <QStack>

class GraphicsKeyItem : public QGraphicsPolygonItem
{
public:
    enum Type {
        BLACK,
        WHITE
    };
    GraphicsKeyItem(unsigned char noteNumber, const QSizeF &keySize, QGraphicsItem *parent = 0);

    Type getType() const;

    void pressKey();
    void releaseKey();

private:
    unsigned char noteNumber;
    QSizeF whiteKeySize, blackKeySize;
    QStack<QBrush> oldBrushes;
    Type type;

    QRectF getBoundingRectangle(unsigned char noteNumber, bool *isBlack = 0);
};

#endif // GRAPHICSKEYITEM_H
