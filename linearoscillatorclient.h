#ifndef LINEAROSCILLATORCLIENT_H
#define LINEAROSCILLATORCLIENT_H

#include "oscillatorclient.h"
#include "linearoscillator.h"
#include "graphicsinterpolatoredititem.h"
#include <QPen>

class LinearOscillatorClient : public OscillatorClient
{
public:
    LinearOscillatorClient(const QString &clientName, size_t ringBufferSize = 1024);
    virtual ~LinearOscillatorClient();

    virtual JackClientFactory * getFactory();

    LinearOscillator * getLinearOscillator();
    LinearInterpolator * getLinearInterpolator();

    void postIncreaseControlPoints();
    void postDecreaseControlPoints();
    void postChangeControlPoint(int index, double x, double y);

    QGraphicsItem * createGraphicsItem(const QRectF &rect);

protected:
    virtual void processEvent(const RingBufferEvent *event, jack_nframes_t time);

private:
    LinearInterpolator interpolator;
};

class LinearOscillatorGraphicsItem : public GraphicsInterpolatorEditItem
{
    Q_OBJECT
public:
    LinearOscillatorGraphicsItem(const QRectF &rect, LinearOscillatorClient *client, QGraphicsItem *parent = 0);

protected:
    virtual void increaseControlPoints();
    virtual void decreaseControlPoints();
    virtual void changeControlPoint(int index, int nrOfControlPoints, double x, double y);

private:
    LinearOscillatorClient *client;
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
