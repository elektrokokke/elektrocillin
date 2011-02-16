#ifndef LINEAROSCILLATORCLIENT_H
#define LINEAROSCILLATORCLIENT_H

#include "eventprocessorclient.h"
#include "linearoscillator.h"
#include "graphicsinterpolationitem.h"
#include <QObject>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QPen>

class GraphicsNodeItem;

struct LinearOscillatorParameters {
    int controlPoints, index;
    double x, y;
};

class LinearOscillatorClient : public EventProcessorClient<LinearOscillatorParameters>
{
public:
    LinearOscillatorClient(const QString &clientName, size_t ringBufferSize = 1024);
    virtual ~LinearOscillatorClient();

    LinearOscillator * getLinearOscillator();

    const LinearInterpolator & postIncreaseControlPoints();
    const LinearInterpolator & postDecreaseControlPoints();

protected:
    virtual void processEvent(const LinearOscillatorParameters &event, jack_nframes_t time);

private:
    LinearInterpolator interpolator;
};

class LinearOscillatorGraphicsItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    LinearOscillatorGraphicsItem(const QRectF &rect, LinearOscillatorClient *client, QGraphicsItem *parent = 0);

public slots:
    void increaseControlPoints();
    void decreaseControlPoints();

protected:
    virtual void mousePressEvent ( QGraphicsSceneMouseEvent * event );

private slots:
    void onNodePositionChangedScaled(QPointF position);

private:
    LinearOscillatorClient *client;
    QMap<QObject*, int> mapSenderToControlPointIndex;
    QVector<GraphicsNodeItem*> nodes;
    LinearInterpolator interpolator;
    LinearIntegralInterpolator interpolatorIntegral;
    GraphicsInterpolationItem *interpolationItem, *interpolationIntegralItem;
    QMenu contextMenu;

    GraphicsNodeItem * createNode(qreal x, qreal y);
};

class PulseWaveGraphicsItem : public QGraphicsRectItem {
public:
    PulseWaveGraphicsItem(const QRectF &rectangle, QGraphicsItem *parent = 0) :
        QGraphicsRectItem(rectangle, parent)
    {
        setPen(QPen(Qt::NoPen));
        setBrush(QBrush(Qt::white));
        QPainterPath path;
        path.moveTo(0, rectangle.height() * 0.5);
        path.lineTo(rectangle.topLeft());
        path.lineTo(rectangle.width() * 0.5, 0);
        path.lineTo(rectangle.width() * 0.5, rectangle.bottom());
        path.lineTo(rectangle.bottomRight());
        path.lineTo(rectangle.width(), rectangle.height() * 0.5);
        (new QGraphicsPathItem(path, this))->setPen(QPen(QBrush(Qt::black), 2));
    }
};

class SawtoothWaveGraphicsItem : public QGraphicsRectItem {
public:
    SawtoothWaveGraphicsItem(const QRectF &rectangle, QGraphicsItem *parent = 0) :
        QGraphicsRectItem(rectangle, parent)
    {
        setPen(QPen(Qt::NoPen));
        setBrush(QBrush(Qt::white));
        QPainterPath path;
        path.moveTo(0, rectangle.height() * 0.5);
        path.lineTo(rectangle.width() * 0.5, 0);
        path.lineTo(rectangle.width() * 0.5, rectangle.bottom());
        path.lineTo(rectangle.width(), rectangle.height() * 0.5);
        (new QGraphicsPathItem(path, this))->setPen(QPen(QBrush(Qt::black), 2));
    }
};

class TriangleWaveGraphicsItem : public QGraphicsRectItem {
public:
    TriangleWaveGraphicsItem(const QRectF &rectangle, QGraphicsItem *parent = 0) :
        QGraphicsRectItem(rectangle, parent)
    {
        setPen(QPen(Qt::NoPen));
        setBrush(QBrush(Qt::white));
        QPainterPath path;
        path.moveTo(0, rectangle.height() * 0.5);
        path.lineTo(rectangle.width() * 0.25, 0);
        path.lineTo(rectangle.width() * 0.75, rectangle.bottom());
        path.lineTo(rectangle.width(), rectangle.height() * 0.5);
        (new QGraphicsPathItem(path, this))->setPen(QPen(QBrush(Qt::black), 2));
    }
};

#endif // LINEAROSCILLATORCLIENT_H
