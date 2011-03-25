#include "polynomialoscillatorclient.h"

PolynomialOscillatorClient::PolynomialOscillatorClient(const QString &clientName, size_t ringBufferSize) :
    OscillatorClient(clientName, new PolynomialOscillator(3), ringBufferSize),
    oscillator(3)
{
}

PolynomialOscillatorClient::~PolynomialOscillatorClient()
{
    close();
}

void PolynomialOscillatorClient::saveState(QDataStream &stream)
{
    OscillatorClient::saveState(stream);
    oscillator.getPolynomialInterpolator()->save(stream);
}

void PolynomialOscillatorClient::loadState(QDataStream &stream)
{
    OscillatorClient::loadState(stream);
    PolynomialInterpolator interpolator;
    interpolator.load(stream);
    oscillator.setPolynomialInterpolator(interpolator);
    getIntegralOscillator()->setPolynomialInterpolator(interpolator);
}

PolynomialInterpolator * PolynomialOscillatorClient::getPolynomialInterpolator()
{
    return oscillator.getPolynomialInterpolator();
}

void PolynomialOscillatorClient::postIncreaseControlPoints()
{
    InterpolatorProcessor::AddControlPointsEvent *event = new InterpolatorProcessor::AddControlPointsEvent(true, false, false, true);
    oscillator.processEvent(event, 0);
    postEvent(event);
}

void PolynomialOscillatorClient::postDecreaseControlPoints()
{
    if (oscillator.getPolynomialInterpolator()->getX().size() > 2) {
        InterpolatorProcessor::DeleteControlPointsEvent *event = new InterpolatorProcessor::DeleteControlPointsEvent(true, false, false, true);
        oscillator.processEvent(event, 0);
        postEvent(event);
    }
}

void PolynomialOscillatorClient::postChangeControlPoint(int index, double x, double y)
{
    InterpolatorProcessor::ChangeControlPointEvent *event = new InterpolatorProcessor::ChangeControlPointEvent(index, x, y);
    oscillator.processEvent(event, 0);
    postEvent(event);
}

PolynomialOscillator * PolynomialOscillatorClient::getIntegralOscillator()
{
    return (PolynomialOscillator*)getAudioProcessor();
}

QGraphicsItem * PolynomialOscillatorClient::createGraphicsItem()
{
    QGraphicsPathItem *pathItem = new QGraphicsPathItem();
    QGraphicsItem *oscillatorItem = OscillatorClient::createGraphicsItem();
    QRectF rect = QRect(0, 0, 600, 420).translated(oscillatorItem->boundingRect().width(), 0);
    oscillatorItem->setParentItem(pathItem);
    (new IntegralOscillatorGraphicsItem(rect, this))->setParentItem(pathItem);
    QPainterPath path;
    path.addRect(rect);
    path.addRect(oscillatorItem->boundingRect().translated(oscillatorItem->pos()));
    pathItem->setPen(QPen(Qt::NoPen));
    pathItem->setPath(path);
    return pathItem;
}

IntegralOscillatorGraphicsItem::IntegralOscillatorGraphicsItem(const QRectF &rect, PolynomialOscillatorClient *client_, QGraphicsItem *parent) :
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
        return "Polynomial oscillator";
    }
    JackClient * createClient(const QString &clientName)
    {
        return new PolynomialOscillatorClient(clientName);
    }
    static IntegralOscillatorClientFactory factory;
};

IntegralOscillatorClientFactory IntegralOscillatorClientFactory::factory;

JackClientFactory * PolynomialOscillatorClient::getFactory()
{
    return &IntegralOscillatorClientFactory::factory;
}
