#ifndef LINEARWAVESHAPINGCLIENT_H
#define LINEARWAVESHAPINGCLIENT_H

#include "eventprocessorclient.h"
#include "linearinterpolator.h"
#include <QGraphicsRectItem>
#include <QObject>

struct LinearWaveShapingParameters {
    double x, y;
};

class LinearWaveShapingClient : public EventProcessorClient<LinearWaveShapingParameters>
{
public:
    LinearWaveShapingClient(const QString &clientName, size_t ringBufferSize = 1024);
    virtual ~LinearWaveShapingClient();

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
    void onNodePositionChanged(QPointF position);

private:
    LinearWaveShapingClient *client;
};

#endif // LINEARWAVESHAPINGCLIENT_H
