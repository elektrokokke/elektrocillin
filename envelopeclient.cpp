#include "envelopeclient.h"
#include <cmath>

EnvelopeClient::EnvelopeClient(const QString &clientName, size_t ringBufferSize) :
    EventProcessorClient(clientName, new Envelope(), ringBufferSize)
{
    envelopeProcess = (Envelope*)getAudioProcessor();
    envelope = new Envelope();
    envelope->copyInterpolator(envelopeProcess);
}

EnvelopeClient::~EnvelopeClient()
{
    close();
    delete envelope;
    delete envelopeProcess;
}

void EnvelopeClient::saveState(QDataStream &stream)
{
    envelope->save(stream);
}

void EnvelopeClient::loadState(QDataStream &stream)
{
    envelopeProcess->load(stream);
    envelope->copyInterpolator(envelopeProcess);
}

Envelope * EnvelopeClient::getEnvelope()
{
    return envelope;
}

void EnvelopeClient::postIncreaseControlPoints()
{
    Interpolator::ChangeAllControlPointsEvent *event = envelope->createIncreaseControlPointsEvent();
    envelope->processEvent(event, 0);
    postEvent(event);
}

void EnvelopeClient::postDecreaseControlPoints()
{
    if (envelope->getInterpolator()->getX().size() > 2) {
        Interpolator::ChangeAllControlPointsEvent *event = envelope->createDecreaseControlPointsEvent();
        envelope->processEvent(event, 0);
        postEvent(event);
    }
}

void EnvelopeClient::postChangeControlPoint(int index, double x, double y)
{
    Interpolator *interpolator = envelope->getInterpolator();
    if (index == 0) {
       x = interpolator->getX()[0];
    }
    if (index == interpolator->getX().size() - 1) {
        y = 0;
    }
    if ((index > 0) && (x <= interpolator->getX()[index - 1])) {
        x = interpolator->getX()[index - 1];
    }
    if ((index < interpolator->getX().size() - 1) && (x >= interpolator->getX()[index + 1])) {
        x = interpolator->getX()[index + 1];
    }
    Interpolator::ChangeControlPointEvent *event = new Interpolator::ChangeControlPointEvent();
    event->index = index;
    event->x = x;
    event->y = y;
    envelope->processEvent(event, 0);
    postEvent(event);
}

void EnvelopeClient::postChangeSustainIndex(int sustainIndex)
{
    Envelope::ChangeSustainPositionEvent *event = new Envelope::ChangeSustainPositionEvent();
    event->sustainIndex = sustainIndex;
    envelope->processEvent(event, 0);
    postEvent(event);
}

QGraphicsItem * EnvelopeClient::createGraphicsItem()
{
    return new EnvelopeGraphicsItem(QRectF(0, 0, 1200, 420), this);
}

void EnvelopeClient::processEvent(const RingBufferEvent *event, jack_nframes_t time)
{
    if (const Interpolator::ChangeControlPointEvent *changeControlPointEvent = dynamic_cast<const Interpolator::ChangeControlPointEvent*>(event)) {
        envelopeProcess->processEvent(changeControlPointEvent, time);
    } else if (const Interpolator::ChangeAllControlPointsEvent *changeAllControlPointsEvent = dynamic_cast<const Interpolator::ChangeAllControlPointsEvent*>(event)) {
        envelopeProcess->processEvent(changeAllControlPointsEvent, time);
    } else if (const Envelope::ChangeSustainPositionEvent *changeSustainPositionEvent = dynamic_cast<const Envelope::ChangeSustainPositionEvent*>(event)) {
        envelopeProcess->processEvent(changeSustainPositionEvent, time);
    }
}

EnvelopeGraphicsItem::EnvelopeGraphicsItem(const QRectF &rect, EnvelopeClient *client_, QGraphicsItem *parent_) :
    GraphicsInterpolatorEditItem(client_->getEnvelope()->getInterpolator(),
        rect,
        QRectF(0, 1, log(client_->getEnvelope()->getDurationInSeconds() + 1), -2),
        parent_,
        4,
        8,
        true
        ),
    client(client_)
{
    setVisible(GraphicsInterpolatorEditItem::FIRST, false);
}

EnvelopeClient * EnvelopeGraphicsItem::getClient()
{
    return client;
}

void EnvelopeGraphicsItem::increaseControlPoints()
{
    client->postIncreaseControlPoints();
}

void EnvelopeGraphicsItem::decreaseControlPoints()
{
    client->postDecreaseControlPoints();
}

void EnvelopeGraphicsItem::changeControlPoint(int index, double x, double y)
{
    client->postChangeControlPoint(index, x, y);
}

class EnvelopeClientFactory : public JackClientFactory
{
public:
    EnvelopeClientFactory()
    {
        JackClientSerializer::getInstance()->registerFactory(this);
    }
    QString getName()
    {
        return "Envelope";
    }
    JackClient * createClient(const QString &clientName)
    {
        return new EnvelopeClient(clientName);
    }
    static EnvelopeClientFactory factory;
};

EnvelopeClientFactory EnvelopeClientFactory::factory;

JackClientFactory * EnvelopeClient::getFactory()
{
    return &EnvelopeClientFactory::factory;
}
