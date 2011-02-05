#ifndef SPLINEWAVESHAPINGCLIENT_H
#define SPLINEWAVESHAPINGCLIENT_H

#include "eventprocessorclient.h"
#include "cubicsplineinterpolator.h"
#include "graphicsinterpolationitem.h"
#include <QObject>
#include <QGraphicsRectItem>

struct CubicSplineWaveShapingParameters {
    double x[3], y[3], y2[3];
};

class CubicSplineWaveShapingClient : public EventProcessorClient<CubicSplineWaveShapingParameters>
{
public:
    CubicSplineWaveShapingClient(const QString &clientName, size_t ringBufferSize = 1024);
    virtual ~CubicSplineWaveShapingClient();

    const CubicSplineInterpolator & getInterpolator() const;

protected:
    virtual void processAudio(const double *inputs, double *outputs, jack_nframes_t time);
    virtual void processEvent(const CubicSplineWaveShapingParameters &event, jack_nframes_t time);

private:
    CubicSplineInterpolator interpolator;
};

class CubicSplineWaveShapingGraphicsItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    CubicSplineWaveShapingGraphicsItem(const QRectF &rect, CubicSplineWaveShapingClient *client, QGraphicsItem *parent = 0);

private slots:
    void onNodePositionChanged(QPointF position);

private:
    CubicSplineWaveShapingClient *client;
    CubicSplineInterpolator interpolator;
    GraphicsInterpolationItem *interpolationItem;
};

#endif // SPLINEWAVESHAPINGCLIENT_H
