#ifndef GRAPHICSMETERITEM_H
#define GRAPHICSMETERITEM_H

#include "graphicslabelitem.h"
#include "graphicsnodeitem.h"
#include <QGraphicsPathItem>
#include <QGraphicsLineItem>
#include <QPen>
#include <QBrush>

class GraphicsMeterItem : public QObject, public QGraphicsPathItem
{
    Q_OBJECT
public:
    enum EllipsisHalf {
        TOP_HALF,
        BOTTOM_HALF
    };
    GraphicsMeterItem(const QRectF &rect, const QString &name, double minValue, double maxValue, double value, int slices, int valuesPerSlice = 10, EllipsisHalf half = TOP_HALF, QGraphicsItem *parent = 0);

    void setRange(double minValue, double maxValue, double value, int slices);

public slots:
    void setValue(double value);
signals:
    void valueChanged(double value);
private slots:
    void onNodePositionChangedScaled(QPointF pos);
private:
    QString name;
    double minValue, maxValue, value;
    int slices, valuesPerSlice;
    EllipsisHalf half;
    int verticalPadding;
    QPen nodePen;
    QBrush nodeBrush;
    QRectF innerRect;
    QPointF innerCenter;
    GraphicsNodeItem *nodeItem;
    GraphicsLabelItem *labelItem;
    QGraphicsLineItem *needleItem;
    QVector<QGraphicsItem*> ticks;

    void initTicks();
};

#endif // GRAPHICSMETERITEM_H
