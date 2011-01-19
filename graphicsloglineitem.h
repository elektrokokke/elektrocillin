#ifndef GRAPHICSLOGLINEITEM_H
#define GRAPHICSLOGLINEITEM_H

#include <QGraphicsPathItem>

class GraphicsLogLineItem : public QObject, public QGraphicsPathItem
{
    Q_OBJECT
public:
    explicit GraphicsLogLineItem(const QPointF &p1, const QPointF &p2, bool logarithmic = false, QGraphicsItem * parent = 0);
    explicit GraphicsLogLineItem(qreal x1, qreal y1, qreal x2, qreal y2, bool logarithmic = false, QGraphicsItem * parent = 0);

public slots:
    void setP1(QPointF point);
    void setP2(QPointF point);
    void setLogarithmic(bool logarithmic);

private:
    QPainterPath constructPath();

    QPointF p1, p2;
    bool logarithmic;
};

#endif // GRAPHICSLOGLINEITEM_H
