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
    virtual void saveState(QDataStream &stream);
    virtual void loadState(QDataStream &stream);

    LinearInterpolator * getLinearInterpolator();

    void postIncreaseControlPoints();
    void postDecreaseControlPoints();
    void postChangeControlPoint(int index, double x, double y);

    QGraphicsItem * createGraphicsItem();
protected:
    LinearOscillator * getLinearOscillator();
private:
    LinearInterpolator interpolator;
};

class LinearOscillatorGraphicsItem : public GraphicsInterpolatorEditItem
{
public:
    LinearOscillatorGraphicsItem(const QRectF &rect, LinearOscillatorClient *client, QGraphicsItem *parent = 0);

protected:
    virtual void increaseControlPoints();
    virtual void decreaseControlPoints();
    virtual void changeControlPoint(int index, double x, double y);

private:
    LinearOscillatorClient *client;
};

#endif // LINEAROSCILLATORCLIENT_H
