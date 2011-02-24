#include "envelopeclient.h"

EnvelopeClient::EnvelopeClient(const QString &clientName, size_t ringBufferSize) :
        EventProcessorClient2(clientName, new Envelope(), ringBufferSize)
{
}

EnvelopeClient::~EnvelopeClient()
{
    delete getAudioProcessor();
}

Envelope * EnvelopeClient::getEnvelope()
{
    return (Envelope*)getAudioProcessor();
}

void EnvelopeClient::postChangeDuration(double duration)
{
    ChangeDurationEvent *event = new ChangeDurationEvent();
    event->duration = duration;
    postEvent(event);
}

void EnvelopeClient::processEvent(const RingBufferEvent *event, jack_nframes_t time)
{
    if (const ChangeDurationEvent *changeDurationEvent = dynamic_cast<const ChangeDurationEvent*>(event)) {
        getEnvelope()->setDuration(changeDurationEvent->duration);
    }
}

EnvelopeGraphicsItem::EnvelopeGraphicsItem(const QRectF &rect, EnvelopeClient *client_, QGraphicsItem *parent, const QPen &nodePen, const QBrush &nodeBrush) :
        QGraphicsRectItem(rect, parent),
        client(client_)
{
    interpolator[Envelope::ATTACK] = client->getEnvelope()->getInterpolator(Envelope::ATTACK);
    interpolator[Envelope::RELEASE] = client->getEnvelope()->getInterpolator(Envelope::RELEASE);
    // create two interpolator edit items inside the rectangle:
    double width = rect.width() - 16;
    double attackWidth = width * interpolator[]
    attackItem = new GraphicsInterpolatorEditItem(&interpolator[Envelope::ATTACK], )
}
