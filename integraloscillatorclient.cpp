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
    Interpolator::AddControlPointsEvent *event = new Interpolator::AddControlPointsEvent(true, false, false, true);
    oscillator.processEvent(event, 0);
    postEvent(event);
}

void IntegralOscillatorClient::postDecreaseControlPoints()
{
    if (oscillator.getPolynomialInterpolator()->getX().size() > 2) {
        Interpolator::DeleteControlPointsEvent *event = new Interpolator::DeleteControlPointsEvent(true, false, false, true);
        oscillator.processEvent(event, 0);
        postEvent(event);
    }
}

void IntegralOscillatorClient::postChangeControlPoint(int index, double x, double y)
{
    Interpolator::ChangeControlPointEvent *event = new Interpolator::ChangeControlPointEvent(index, x, y);
    oscillator.processEvent(event, 0);
    postEvent(event);
}

QGraphicsItem * IntegralOscillatorClient::createGraphicsItem()
{
    QRectF rect(0, 0, 600, 420);
    QGraphicsRectItem *rectItem = new QGraphicsRectItem(rect);
    rectItem->setPen(QPen(Qt::NoPen));
    QRectF rectGain(rect.x(), rect.y(), 16, rect.height());
    QRectF rectOscillator = rect.adjusted(rectGain.width(), 0, 0, 0);
    (new OscillatorClientGraphicsItem(rectGain, this))->setParentItem(rectItem);
    (new IntegralOscillatorGraphicsItem(rectOscillator, this))->setParentItem(rectItem);
    return rectItem;
}

bool IntegralOscillatorClient::processEvent(const RingBufferEvent *event, jack_nframes_t time)
{
    return getIntegralOscillator()->processEvent(event, time);
}

IntegralOscillatorGraphicsItem::IntegralOscillatorGraphicsItem(const QRectF &rect, IntegralOscillatorClient *client_, QGraphicsItem *parent) :
    GraphicsInterpolatorEditItem(
        client_->getPolynomialInterpolator(),
        rect,
        QRectF(client_->getPolynomialInterpolator()->getX().first(), 1, client_->getPolynomialInterpolator()->getX().last() - client_->getPolynomialInterpolator()->getX().first(), -2),
        parent
        ),
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
