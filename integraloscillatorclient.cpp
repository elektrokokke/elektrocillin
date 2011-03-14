#include "integraloscillatorclient.h"

IntegralOscillatorClient::IntegralOscillatorClient(const QString &clientName, size_t ringBufferSize) :
    OscillatorClient(clientName, new IntegralOscillator(3), ringBufferSize),
    oscillator(3)
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
    PolynomialInterpolator interpolator;
    interpolator.load(stream);
    oscillator.setPolynomialInterpolator(interpolator);
    getIntegralOscillator()->setPolynomialInterpolator(interpolator);
}

PolynomialInterpolator * IntegralOscillatorClient::getPolynomialInterpolator()
{
    return oscillator.getPolynomialInterpolator();
}

void IntegralOscillatorClient::postIncreaseControlPoints()
{
    InterpolatorProcessor::AddControlPointsEvent *event = new InterpolatorProcessor::AddControlPointsEvent(true, false, false, true);
    oscillator.processEvent(event, 0);
    postEvent(event);
}

void IntegralOscillatorClient::postDecreaseControlPoints()
{
    if (oscillator.getPolynomialInterpolator()->getX().size() > 2) {
        InterpolatorProcessor::DeleteControlPointsEvent *event = new InterpolatorProcessor::DeleteControlPointsEvent(true, false, false, true);
        oscillator.processEvent(event, 0);
        postEvent(event);
    }
}

void IntegralOscillatorClient::postChangeControlPoint(int index, double x, double y)
{
    InterpolatorProcessor::ChangeControlPointEvent *event = new InterpolatorProcessor::ChangeControlPointEvent(index, x, y);
    oscillator.processEvent(event, 0);
    postEvent(event);
}

IntegralOscillator * IntegralOscillatorClient::getIntegralOscillator()
{
    return (IntegralOscillator*)getAudioProcessor();
}

QGraphicsItem * IntegralOscillatorClient::createGraphicsItem()
{
    QRectF rect(0, 0, 600, 420);
    QGraphicsPathItem *pathItem = new QGraphicsPathItem();
    QGraphicsItem *oscillatorItem = OscillatorClient::createGraphicsItem();
    oscillatorItem->setParentItem(pathItem);
    oscillatorItem->setPos(rect.left() - oscillatorItem->boundingRect().width(), rect.bottom() - oscillatorItem->boundingRect().height());
    (new IntegralOscillatorGraphicsItem(rect, this))->setParentItem(pathItem);
    QPainterPath path;
    path.addRect(rect);
    path.addRect(oscillatorItem->boundingRect().translated(oscillatorItem->pos()));
    pathItem->setPen(QPen(Qt::NoPen));
    pathItem->setPath(path);
    return pathItem;
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
