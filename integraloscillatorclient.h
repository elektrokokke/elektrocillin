#ifndef INTEGRALOSCILLATORCLIENT_H
#define INTEGRALOSCILLATORCLIENT_H

#include "oscillatorclient.h"
#include "integraloscillator.h"
#include "graphicsinterpolatoredititem.h"
#include <QPen>


class IntegralOscillatorClient : public OscillatorClient
{
public:
    IntegralOscillatorClient(const QString &clientName, size_t ringBufferSize = 1024);
    virtual ~IntegralOscillatorClient();

    virtual JackClientFactory * getFactory();
    virtual void saveState(QDataStream &stream);
    virtual void loadState(QDataStream &stream);

    IntegralOscillator * getIntegralOscillator();
    PolynomialInterpolator * getPolynomialInterpolator();

    void postIncreaseControlPoints();
    void postDecreaseControlPoints();
    void postChangeControlPoint(int index, double x, double y);

    QGraphicsItem * createGraphicsItem();

protected:
    virtual void processEvent(const RingBufferEvent *event, jack_nframes_t time);

private:
    IntegralOscillator oscillator;
};

class IntegralOscillatorGraphicsItem : public GraphicsInterpolatorEditItem
{
public:
    IntegralOscillatorGraphicsItem(const QRectF &rect, IntegralOscillatorClient *client, QGraphicsItem *parent = 0);

protected:
    virtual void increaseControlPoints();
    virtual void decreaseControlPoints();
    virtual void changeControlPoint(int index, double x, double y);

private:
    IntegralOscillatorClient *client;
};

#endif // INTEGRALOSCILLATORCLIENT_H
