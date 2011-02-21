#ifndef LINEARMORPHOSCILLATORCLIENT_H
#define LINEARMORPHOSCILLATORCLIENT_H

#include "eventprocessorclient.h"
#include "linearmorphoscillator.h"
#include "graphicsinterpolatoredititem.h"

class LinearMorphOscillatorClient : public EventProcessorClient<LinearMorphOscillatorParameters>
{
public:
    LinearMorphOscillatorClient(const QString &clientName, double frequencyModulationIntensity = 2.0/12.0, size_t ringBufferSize = 1024);
    virtual ~LinearMorphOscillatorClient();

    LinearMorphOscillator * getLinearMorphOscillator();
    LinearInterpolator * getState(int state);

    void postIncreaseControlPoints();
    void postDecreaseControlPoints();
    void postChangeControlPoint(int state, int index, int nrOfControlPoints, double x, double y);

protected:
    virtual void processEvent(const LinearMorphOscillatorParameters &event, jack_nframes_t time);

private:
    LinearInterpolator state[2];
};

class LinearMorphOscillatorGraphicsSubItem : public GraphicsInterpolatorEditItem
{
    Q_OBJECT
public:
    LinearMorphOscillatorGraphicsSubItem(const QRectF &rect, LinearMorphOscillatorClient *client, int state, QGraphicsItem *parent = 0);

    void setTwin(LinearMorphOscillatorGraphicsSubItem *twin);

protected:
    virtual void increaseControlPoints();
    virtual void decreaseControlPoints();
    virtual void changeControlPoint(int index, int nrOfControlPoints, double x, double y);

private:
    LinearMorphOscillatorClient *client;
    int state;
    LinearMorphOscillatorGraphicsSubItem *twin;
};

class LinearMorphOscillatorGraphicsItem : public QGraphicsRectItem{
public:
    LinearMorphOscillatorGraphicsItem(const QRectF &rect, LinearMorphOscillatorClient *client, QGraphicsItem *parent = 0);
};


#endif // LINEARMORPHOSCILLATORCLIENT_H
