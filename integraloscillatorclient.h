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
    /**
      To call this method is only safe when the client is not running,
      as it accesses the internal oscillator object used by the Jack
      process thread in a non-threadsafe way.

      To change the oscillator parameters while the client is running use
      post...() methods.
      */
    virtual void loadState(QDataStream &stream);

    PolynomialInterpolator * getPolynomialInterpolator();

    void postIncreaseControlPoints();
    void postDecreaseControlPoints();
    void postChangeControlPoint(int index, double x, double y);

    QGraphicsItem * createGraphicsItem();
protected:
    IntegralOscillator * getIntegralOscillator();
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
