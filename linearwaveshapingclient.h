#ifndef LINEARWAVESHAPINGCLIENT_H
#define LINEARWAVESHAPINGCLIENT_H

#include "eventprocessorclient.h"
#include "linearinterpolator.h"
#include <QGraphicsRectItem>
#include <QObject>
#include <QMap>

struct LinearWaveShapingParameters {
    double x[5], y[5];
};

class LinearWaveShapingClient : public EventProcessorClient<LinearWaveShapingParameters>
{
public:
    LinearWaveShapingClient(const QString &clientName, size_t ringBufferSize = 1024);
    virtual ~LinearWaveShapingClient();

    const LinearWaveShapingParameters & getParameters() const;

    static const int controlPointCount;

protected:
    virtual void processAudio(const double *inputs, double *outputs, jack_nframes_t time);
    virtual void processEvent(const LinearWaveShapingParameters &event, jack_nframes_t time);

private:
    LinearWaveShapingParameters parameters;
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
    LinearWaveShapingParameters parameters;
    QMap<QObject*, int> mapSenderToControlPointIndex;
};

#endif // LINEARWAVESHAPINGCLIENT_H
