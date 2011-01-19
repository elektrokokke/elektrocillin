#ifndef GRAPHICSLINEITEM_H
#define GRAPHICSLINEITEM_H

#include <QObject>
#include <QGraphicsLineItem>

class GraphicsLineItem : public QObject, public QGraphicsLineItem
{
    Q_OBJECT
public:
    explicit GraphicsLineItem(QGraphicsItem *parent = 0);
    explicit GraphicsLineItem(const QLineF & line, QGraphicsItem * parent = 0);
    explicit GraphicsLineItem(qreal x1, qreal y1, qreal x2, qreal y2, QGraphicsItem * parent = 0);

    enum EndPoints {
        P1,
        P2
    };

public slots:
    void setP1(QPointF point);
    void setP2(QPointF point);

};

#endif // GRAPHICSLINEITEM_H
