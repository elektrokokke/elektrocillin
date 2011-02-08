#ifndef LINEAROSCILLATORCLIENT_H
#define LINEAROSCILLATORCLIENT_H

#include "eventprocessorclient.h"
#include "linearoscillator.h"
#include "graphicsinterpolationitem.h"
#include <QObject>
#include <QGraphicsRectItem>

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

private slots:
    void onNodePositionChangedScaled(QPointF position);

private:
    LinearOscillatorClient *client;
    QMap<QObject*, int> mapSenderToControlPointIndex;
    LinearInterpolator interpolator;
    LinearIntegralInterpolator interpolatorIntegral;
    GraphicsInterpolationItem *interpolationItem, *interpolationIntegralItem;
};


#endif // LINEAROSCILLATORCLIENT_H
