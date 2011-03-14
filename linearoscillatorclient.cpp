#include "linearoscillatorclient.h"
#include <cmath>

LinearOscillatorClient::LinearOscillatorClient(const QString &clientName, size_t ringBufferSize) :
    OscillatorClient(clientName, new LinearOscillator(), ringBufferSize),
    interpolator(getLinearOscillator()->getLinearInterpolator())
{
}

LinearOscillatorClient::~LinearOscillatorClient()
{
    close();
}

void LinearOscillatorClient::saveState(QDataStream &stream)
{
    OscillatorClient::saveState(stream);
    interpolator.save(stream);
}

void LinearOscillatorClient::loadState(QDataStream &stream)
{
    OscillatorClient::loadState(stream);
    interpolator.load(stream);
    getLinearOscillator()->setLinearInterpolator(interpolator);
}

LinearInterpolator * LinearOscillatorClient::getLinearInterpolator()
{
    return &interpolator;
}

void LinearOscillatorClient::postIncreaseControlPoints()
{
    InterpolatorProcessor::AddControlPointsEvent *event = new InterpolatorProcessor::AddControlPointsEvent(true, false, false, true);
    interpolator.addControlPoints(true, false, false, true);
    postEvent(event);
}

void LinearOscillatorClient::postDecreaseControlPoints()
{
    if (interpolator.getX().size() > 2) {
        InterpolatorProcessor::DeleteControlPointsEvent *event = new InterpolatorProcessor::DeleteControlPointsEvent(true, false, false, true);
        interpolator.deleteControlPoints(true, false, false, true);
        postEvent(event);
    }
}

void LinearOscillatorClient::postChangeControlPoint(int index, double x, double y)
{
    InterpolatorProcessor::ChangeControlPointEvent *event = new InterpolatorProcessor::ChangeControlPointEvent(index, x, y);
    interpolator.changeControlPoint(index, x, y);
    postEvent(event);
}

LinearOscillator * LinearOscillatorClient::getLinearOscillator()
{
    return (LinearOscillator*)getAudioProcessor();
}

QGraphicsItem * LinearOscillatorClient::createGraphicsItem()
{
    QRectF rect(0, 0, 600, 420);
    QGraphicsPathItem *pathItem = new QGraphicsPathItem();
    QGraphicsItem *oscillatorItem = OscillatorClient::createGraphicsItem();
    oscillatorItem->setParentItem(pathItem);
    oscillatorItem->setPos(rect.left() - oscillatorItem->boundingRect().width(), rect.bottom() - oscillatorItem->boundingRect().height());
    (new LinearOscillatorGraphicsItem(rect, this))->setParentItem(pathItem);
    QPainterPath path;
    path.addRect(rect);
    path.addRect(oscillatorItem->boundingRect().translated(oscillatorItem->pos()));
    pathItem->setPen(QPen(Qt::NoPen));
    pathItem->setPath(path);
    return pathItem;
}

LinearOscillatorGraphicsItem::LinearOscillatorGraphicsItem(const QRectF &rect, LinearOscillatorClient *client_, QGraphicsItem *parent) :
    GraphicsInterpolatorEditItem(client_->getLinearInterpolator(),
        rect,
        QRectF(client_->getLinearInterpolator()->getX().first(), 1, client_->getLinearInterpolator()->getX().last() - client_->getLinearInterpolator()->getX().first(), -2),
        parent
        ),
    client(client_)
{}

void LinearOscillatorGraphicsItem::increaseControlPoints()
{
    client->postIncreaseControlPoints();
}

void LinearOscillatorGraphicsItem::decreaseControlPoints()
{
    client->postDecreaseControlPoints();
}

void LinearOscillatorGraphicsItem::changeControlPoint(int index, double x, double y)
{
    client->postChangeControlPoint(index, x, y);
}

class LinearOscillatorClientFactory : public JackClientFactory
{
public:
    LinearOscillatorClientFactory()
    {
        JackClientSerializer::getInstance()->registerFactory(this);
    }
    QString getName()
    {
        return "Piecewise linear oscillator";
    }
    JackClient * createClient(const QString &clientName)
    {
        return new LinearOscillatorClient(clientName);
    }
    static LinearOscillatorClientFactory factory;
};

LinearOscillatorClientFactory LinearOscillatorClientFactory::factory;

JackClientFactory * LinearOscillatorClient::getFactory()
{
    return &LinearOscillatorClientFactory::factory;
}
