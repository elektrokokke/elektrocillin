#include "integraloscillatorclient.h"
#include <cmath>

IntegralOscillatorClient::IntegralOscillatorClient(const QString &clientName, size_t ringBufferSize) :
    OscillatorClient(clientName, new IntegralOscillator(), ringBufferSize)
{
}

IntegralOscillatorClient::~IntegralOscillatorClient()
{
    close();
}

void IntegralOscillatorClient::saveState(QDataStream &stream)
{
    OscillatorClient::saveState(stream);
    oscillator.getPolynomialInterpolator()->save(stream);
}

void IntegralOscillatorClient::loadState(QDataStream &stream)
{
    OscillatorClient::loadState(stream);
    oscillator.getPolynomialInterpolator()->load(stream);
    getIntegralOscillator()->setPolynomialInterpolator(*oscillator.getPolynomialInterpolator());
}

IntegralOscillator * IntegralOscillatorClient::getIntegralOscillator()
{
    return (IntegralOscillator*)getAudioProcessor();
}

PolynomialInterpolator * IntegralOscillatorClient::getPolynomialInterpolator()
{
    return oscillator.getPolynomialInterpolator();
}

void IntegralOscillatorClient::postIncreaseControlPoints()
{
    Interpolator::ChangeAllControlPointsEvent *event = new Interpolator::ChangeAllControlPointsEvent();
    // TODO: initialize the event
    oscillator.processEvent(event, 0);
    postEvent(event);
}

void IntegralOscillatorClient::postDecreaseControlPoints()
{
    if (oscillator.getPolynomialInterpolator()->getX().size() > 2) {
        Interpolator::ChangeAllControlPointsEvent *event = new Interpolator::ChangeAllControlPointsEvent();
        // TODO: initialize the event
        oscillator.processEvent(event, 0);
        postEvent(event);
    }
}

void IntegralOscillatorClient::postChangeControlPoint(int index, double x, double y)
{
    if (index == 0) {
       x = oscillator.getPolynomialInterpolator()->getX()[0];
    }
    if (index == oscillator.getPolynomialInterpolator()->getX().size() - 1) {
        x = oscillator.getPolynomialInterpolator()->getX().back();
    }
    if ((index > 0) && (x <= oscillator.getPolynomialInterpolator()->getX()[index - 1])) {
        x = oscillator.getPolynomialInterpolator()->getX()[index - 1];
    }
    if ((index < oscillator.getPolynomialInterpolator()->getX().size() - 1) && (x >= oscillator.getPolynomialInterpolator()->getX()[index + 1])) {
        x = oscillator.getPolynomialInterpolator()->getX()[index + 1];
    }
    Interpolator::ChangeControlPointEvent *event = new Interpolator::ChangeControlPointEvent();
    event->index = index;
    event->x = x;
    event->y = y;
    oscillator.processEvent(event, 0);
    postEvent(event);
}

QGraphicsItem * IntegralOscillatorClient::createGraphicsItem(const QRectF &rect)
{
    QGraphicsRectItem *rectItem = new QGraphicsRectItem(rect);
    rectItem->setPen(QPen(Qt::NoPen));
    QRectF rectGain(rect.x(), rect.y(), 16, rect.height());
    QRectF rectOscillator = rect.adjusted(rectGain.width(), 0, 0, 0);
    (new OscillatorClientGraphicsItem(rectGain, this))->setParentItem(rectItem);
    (new IntegralOscillatorGraphicsItem(rectOscillator, this))->setParentItem(rectItem);
    return rectItem;
}

void IntegralOscillatorClient::processEvent(const RingBufferEvent *event, jack_nframes_t time)
{
    if (const Interpolator::ChangeControlPointEvent *changeControlPointEvent = dynamic_cast<const Interpolator::ChangeControlPointEvent*>(event)) {
        getIntegralOscillator()->processEvent(changeControlPointEvent, time);
    } else if (const Interpolator::ChangeAllControlPointsEvent *changeAllControlPointsEvent = dynamic_cast<const Interpolator::ChangeAllControlPointsEvent*>(event)) {
        getIntegralOscillator()->processEvent(changeAllControlPointsEvent, time);
    } else {
        OscillatorClient::processEvent(event, time);
    }
}

IntegralOscillatorGraphicsItem::IntegralOscillatorGraphicsItem(const QRectF &rect, IntegralOscillatorClient *client_, QGraphicsItem *parent) :
    GraphicsInterpolatorEditItem(client_->getPolynomialInterpolator(), rect, QRectF(0, 1, 2.0 * M_PI, -2), parent),
    client(client_)
{}

void IntegralOscillatorGraphicsItem::increaseControlPoints()
{
    client->postIncreaseControlPoints();
}

void IntegralOscillatorGraphicsItem::decreaseControlPoints()
{
    client->postDecreaseControlPoints();
}

void IntegralOscillatorGraphicsItem::changeControlPoint(int index, double x, double y)
{
    client->postChangeControlPoint(index, x, y);
}

class IntegralOscillatorClientFactory : public JackClientFactory
{
public:
    IntegralOscillatorClientFactory()
    {
        JackClientSerializer::getInstance()->registerFactory(this);
    }
    QString getName()
    {
        return "Polynomial integration oscillator";
    }
    JackClient * createClient(const QString &clientName)
    {
        return new IntegralOscillatorClient(clientName);
    }
    static IntegralOscillatorClientFactory factory;
};

IntegralOscillatorClientFactory IntegralOscillatorClientFactory::factory;

JackClientFactory * IntegralOscillatorClient::getFactory()
{
    return &IntegralOscillatorClientFactory::factory;
}
