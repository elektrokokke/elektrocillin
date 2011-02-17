#ifndef SPLINEWAVESHAPINGCLIENT_H
#define SPLINEWAVESHAPINGCLIENT_H

#include "eventprocessorclient.h"
#include "cubicsplineinterpolator.h"
#include "graphicsinterpolatoredititem.h"

class CubicSplineWaveShapingClient : public EventProcessorClient<InterpolatorParameters>
{
public:
    CubicSplineWaveShapingClient(const QString &clientName, size_t ringBufferSize = 1024);
    virtual ~CubicSplineWaveShapingClient();

    CubicSplineInterpolator * getInterpolator();

    void postIncreaseControlPoints();
    void postDecreaseControlPoints();
    void postChangeControlPoint(int index, int nrOfControlPoints, double x, double y);

protected:
    virtual void processAudio(const double *inputs, double *outputs, jack_nframes_t time);
    virtual void processEvent(const InterpolatorParameters &event, jack_nframes_t time);

private:
    CubicSplineInterpolator interpolator, interpolatorProcess;
};

class CubicSplineWaveShapingGraphicsItem : public GraphicsInterpolatorEditItem
{
    Q_OBJECT
public:
    CubicSplineWaveShapingGraphicsItem(CubicSplineWaveShapingClient *client, const QRectF &rect, QGraphicsItem *parent = 0);

protected:
    virtual void increaseControlPoints();
    virtual void decreaseControlPoints();
    virtual void changeControlPoint(int index, int nrOfControlPoints, double x, double y);

private:
    CubicSplineWaveShapingClient *client;
};

#endif // SPLINEWAVESHAPINGCLIENT_H
