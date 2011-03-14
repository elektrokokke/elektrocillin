#include "graphicsmeteritem.h"
#include <cmath>

GraphicsMeterItem::GraphicsMeterItem(const QRectF &rect, const QString &name_, double minValue_, double maxValue_, double value_, int slices, QGraphicsItem *parent) :
    QGraphicsPathItem(parent),
    name(name_),
    minValue(minValue_),
    maxValue(maxValue_),
    value(value_),
    verticalPadding(8),
    nodePen(QPen(QBrush(qRgb(114, 159, 207)), 3)),
    nodeBrush(QBrush(qRgb(52, 101, 164)))
{
    setPen(QPen(Qt::NoPen));

    QPainterPath path(rect.bottomLeft());
    path.lineTo(rect.bottomRight());
    path.lineTo(path.currentPosition() + QPointF(0, -verticalPadding));
    path.arcTo(QRectF(rect.x(), rect.y(), rect.width(), (rect.height() - verticalPadding) * 2), 0, 180);
    path.lineTo(rect.bottomLeft());
    setPath(path);

    innerRect = rect.adjusted(verticalPadding * 0.5 * rect.width() / rect.height(), verticalPadding, -verticalPadding * 0.5 * rect.width() / rect.height(), -verticalPadding);
    innerCenter = QPointF(0.5 * (innerRect.left() + innerRect.right()), innerRect.bottom());

    // draw ticks at regular intervals:
    for (int i = 0; i <= slices * 4; i++) {
        double angle = M_PI * 0.25 * i / slices;
        double x = -cos(angle);
        double y = sin(angle);
        QPointF tickEnd(0.5 * (x + 1) * innerRect.width() + innerRect.left(), innerRect.bottom() - y * innerRect.height());
        QPointF tickStart = (i % 4 == 0 ? 0.8 * tickEnd + 0.2 * innerCenter : 0.9 * tickEnd + 0.1 * innerCenter);
        new QGraphicsLineItem(QLineF(tickEnd, tickStart), this);
    }

    nodeItem = new GraphicsNodeItem(-5, -5, 10, 10, this);
    nodeItem->setPen(nodePen);
    nodeItem->setBrush(nodeBrush);
    nodeItem->setZValue(1);
    nodeItem->setBounds(innerRect);
    nodeItem->setBoundsScaled(QRectF(-1, 1, 2, -1));
    QObject::connect(nodeItem, SIGNAL(positionChangedScaled(QPointF)), this, SLOT(onNodePositionChangedScaled(QPointF)));

    needleItem = new QGraphicsLineItem(this);
    needleItem->setPen(QPen(QBrush(Qt::black), 3));
    labelItem = new GraphicsLabelItem(this);

    setValue(value);
}

void GraphicsMeterItem::setValue(double newValue)
{
    value = qBound(minValue, newValue, maxValue);
    double angle = (value - minValue) / (maxValue - minValue) * M_PI;
    double x = -cos(angle);
    double y = sin(angle);
    nodeItem->setXScaled(x);
    nodeItem->setYScaled(y);
    labelItem->setText(QString("%1: %2").arg(name).arg(value));
    labelItem->setPos(innerRect.bottomLeft() + QPointF(0.5 * (innerRect.width() - labelItem->boundingRect().width()), -labelItem->boundingRect().height()));
    QPointF needleEnd(0.5 * (x + 1) * innerRect.width() + innerRect.left(), innerRect.bottom() - y * innerRect.height());
    needleItem->setLine(QLineF(innerCenter, needleEnd));
}

void GraphicsMeterItem::onNodePositionChangedScaled(QPointF pos)
{
    double angle = M_PI - atan2(pos.y(), pos.x());
    setValue(angle / M_PI * (maxValue - minValue) + minValue);
    valueChanged(value);
}
