#ifndef LINEARWAVESHAPINGCLIENT_H
#define LINEARWAVESHAPINGCLIENT_H

#include "linearinterpolator.h"
#include "eventprocessorclient.h"
#include "graphicsinterpolatoredititem.h"

class LinearWaveShapingClient : public EventProcessorClient<InterpolatorParameters>
{
public:
    LinearWaveShapingClient(const QString &clientName, size_t ringBufferSize = 1024);
    virtual ~LinearWaveShapingClient();

    virtual JackClientFactory * getFactory();
    virtual void saveState(QDataStream &stream);
    virtual void loadState(QDataStream &stream);

    LinearInterpolator * getLinearInterpolator();

    void postIncreaseControlPoints();
    void postDecreaseControlPoints();
    void postChangeControlPoint(int index, int nrOfControlPoints, double x, double y);

    QGraphicsItem * createGraphicsItem(const QRectF &rect);

protected:
    virtual void processAudio(const double *inputs, double *outputs, jack_nframes_t time);
    virtual void processEvent(const InterpolatorParameters &event, jack_nframes_t time);

private:
    LinearInterpolator interpolator, interpolatorProcess;
};

class LinearWaveShapingGraphicsItem : public GraphicsInterpolatorEditItem
{
    Q_OBJECT
public:
    LinearWaveShapingGraphicsItem(const QRectF &rect, LinearWaveShapingClient *client, QGraphicsItem *parent = 0);

protected:
    virtual void increaseControlPoints();
    virtual void decreaseControlPoints();
    virtual void changeControlPoint(int index, int nrOfControlPoints, double x, double y);

private:
    LinearWaveShapingClient *client;
};

#endif // LINEARWAVESHAPINGCLIENT_H
