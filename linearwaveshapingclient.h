#ifndef LINEARWAVESHAPINGCLIENT_H
#define LINEARWAVESHAPINGCLIENT_H

#include "eventprocessorclient.h"
#include "linearintegralinterpolator.h"
#include "graphicsinterpolationitem.h"
#include <QGraphicsRectItem>
#include "graphicslineitem.h"
#include <QObject>
#include <QMap>

struct LinearWaveShapingParameters {
    int controlPoints, index;
    double x, y;
};

class LinearWaveShapingClient : public EventProcessorClient<LinearWaveShapingParameters>
{
public:
    LinearWaveShapingClient(const QString &clientName, size_t ringBufferSize = 1024);
    virtual ~LinearWaveShapingClient();

    const LinearInterpolator & getInterpolator() const;

    static const int controlPointCount;

protected:
    virtual void processAudio(const double *inputs, double *outputs, jack_nframes_t time);
    virtual void processEvent(const LinearWaveShapingParameters &event, jack_nframes_t time);

private:
    LinearInterpolator interpolator;
};

class LinearWaveShapingGraphicsItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    LinearWaveShapingGraphicsItem(const QRectF &rect, LinearWaveShapingClient *client, QGraphicsItem *parent = 0);

private slots:
    void onNodePositionChangedScaled(QPointF position);

private:
    LinearWaveShapingClient *client;
    QMap<QObject*, int> mapSenderToControlPointIndex;
    LinearInterpolator interpolator;
    LinearIntegralInterpolator interpolatorIntegral;
    GraphicsInterpolationItem *interpolationItem, *interpolationIntegralItem;
};

#endif // LINEARWAVESHAPINGCLIENT_H
