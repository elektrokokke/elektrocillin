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
    GraphicsMeterItem(const QRectF &rect, const QString &name, double minValue, double maxValue, double value, int slices, QGraphicsItem *parent = 0);

public slots:
    void setValue(double value);
signals:
    void valueChanged(double value);
private slots:
    void onNodePositionChangedScaled(QPointF pos);
private:
    QString name;
    double minValue, maxValue, value;
    int verticalPadding;
    QPen nodePen;
    QBrush nodeBrush;
    QRectF innerRect;
    QPointF innerCenter;
    GraphicsNodeItem *nodeItem;
    GraphicsLabelItem *labelItem;
    QGraphicsLineItem *needleItem;
};

#endif // GRAPHICSMETERITEM_H
