#ifndef SPLINEWAVESHAPINGCLIENT_H
#define SPLINEWAVESHAPINGCLIENT_H

#include "eventprocessorclient.h"
#include "cubicsplineinterpolator.h"
#include "graphicsinterpolationitem.h"
#include <QObject>
#include <QGraphicsRectItem>

struct CubicSplineWaveShapingParameters {
    double x[4], y[4], y2[4];
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
    void onNodePositionChanged1(QPointF position);
    void onNodePositionChanged2(QPointF position);

private:
    CubicSplineWaveShapingClient *client;
    CubicSplineInterpolator interpolator;
    GraphicsInterpolationItem *interpolationItem;
};

#endif // SPLINEWAVESHAPINGCLIENT_H
