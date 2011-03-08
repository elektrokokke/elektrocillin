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
    Interpolator::ChangeAllControlPointsEvent *event = new Interpolator::ChangeAllControlPointsEvent(interpolator);
    int size = event->xx.size() + 1;
    double stretchFactor = (double)(event->xx.size() - 1) / (double)(size - 1);
    event->xx.append(event->xx.last());
    event->yy.append(event->yy.last());
    for (int i = 0; i < size - 1; i++) {
        event->xx[i] *= stretchFactor;
    }
    interpolator.processEvent(event);
    postEvent(event);
}

void LinearOscillatorClient::postDecreaseControlPoints()
{
    if (interpolator.getX().size() > 2) {
        Interpolator::ChangeAllControlPointsEvent *event = new Interpolator::ChangeAllControlPointsEvent(interpolator);
        int size = event->xx.size() - 1;
        event->xx.resize(size);
        event->yy.resize(size);
        double stretchFactor = 1.0 / event->xx.back();
        for (int i = 0; i < size; i++) {
            event->xx[i] *= stretchFactor;
        }
        interpolator.processEvent(event);
        postEvent(event);
    }
}

void LinearOscillatorClient::postChangeControlPoint(int index, double x, double y)
{
    if (index == 0) {
       x = interpolator.getX()[0];
    }
    if (index == interpolator.getX().size() - 1) {
        x = interpolator.getX().back();
    }
    if ((index > 0) && (x <= interpolator.getX()[index - 1])) {
        x = interpolator.getX()[index - 1];
    }
    if ((index < interpolator.getX().size() - 1) && (x >= interpolator.getX()[index + 1])) {
        x = interpolator.getX()[index + 1];
    }
    Interpolator::ChangeControlPointEvent *event = new Interpolator::ChangeControlPointEvent();
    event->index = index;
    event->x = x;
    event->y = y;
    interpolator.processEvent(event);
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

void LinearOscillatorClient::processEvent(const RingBufferEvent *event, jack_nframes_t time)
{
    if (const Interpolator::ChangeControlPointEvent *changeControlPointEvent = dynamic_cast<const Interpolator::ChangeControlPointEvent*>(event)) {
        getLinearOscillator()->processEvent(changeControlPointEvent, time);
    } else if (const Interpolator::ChangeAllControlPointsEvent *changeAllControlPointsEvent = dynamic_cast<const Interpolator::ChangeAllControlPointsEvent*>(event)) {
        getLinearOscillator()->processEvent(changeAllControlPointsEvent, time);
    } else {
        OscillatorClient::processEvent(event, time);
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
