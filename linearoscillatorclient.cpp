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

LinearOscillator * LinearOscillatorClient::getLinearOscillator()
{
    return (LinearOscillator*)getAudioProcessor();
}

LinearInterpolator * LinearOscillatorClient::getLinearInterpolator()
{
    return &interpolator;
}

void LinearOscillatorClient::postIncreaseControlPoints()
{
    Interpolator::AddControlPointsEvent *event = new Interpolator::AddControlPointsEvent(true, false, false, true);
    interpolator.addControlPoints(event);
    postEvent(event);
}

void LinearOscillatorClient::postDecreaseControlPoints()
{
    if (interpolator.getX().size() > 2) {
        Interpolator::DeleteControlPointsEvent *event = new Interpolator::DeleteControlPointsEvent(true, false, false, true);
        interpolator.deleteControlPoints(event);
        postEvent(event);
    }
}

void LinearOscillatorClient::postChangeControlPoint(int index, double x, double y)
{
    Interpolator::ChangeControlPointEvent *event = new Interpolator::ChangeControlPointEvent(index, x, y);
    interpolator.changeControlPoint(event);
    postEvent(event);
}

QGraphicsItem * LinearOscillatorClient::createGraphicsItem()
{
    QRectF rect(0, 0, 600, 420);
    QGraphicsRectItem *rectItem = new QGraphicsRectItem(rect);
    rectItem->setPen(QPen(Qt::NoPen));
    QRectF rectGain(rect.x(), rect.y(), 16, rect.height());
    QRectF rectOscillator = rect.adjusted(rectGain.width(), 0, 0, 0);
    (new OscillatorClientGraphicsItem(rectGain, this))->setParentItem(rectItem);
    (new LinearOscillatorGraphicsItem(rectOscillator, this))->setParentItem(rectItem);
    return rectItem;
}

bool LinearOscillatorClient::processEvent(const RingBufferEvent *event, jack_nframes_t time)
{
    if (getLinearOscillator()->processEvent(event, time)) {
        return true;
    } else {
        return OscillatorClient::processEvent(event, time);
    }
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
