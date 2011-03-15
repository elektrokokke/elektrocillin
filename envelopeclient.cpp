#include "envelopeclient.h"
#include <cmath>

EnvelopeClient::EnvelopeClient(const QString &clientName, Envelope *envelope_, size_t ringBufferSize) :
    EventProcessorClient(clientName, envelope_, envelope_, envelope_, ringBufferSize),
    envelopeProcess(envelope_)
{
    envelope = new Envelope();
    envelope->copyInterpolatorFrom(envelopeProcess);
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
    *envelope = *envelopeProcess;
}

Envelope * EnvelopeClient::getEnvelope()
{
    return envelope;
}

void EnvelopeClient::postIncreaseControlPoints()
{
    InterpolatorProcessor::AddControlPointsEvent *event = new InterpolatorProcessor::AddControlPointsEvent(true, false, false, true);
    envelope->processEvent(event, 0);
    postEvent(event);
}

void EnvelopeClient::postDecreaseControlPoints()
{
    if (envelope->getInterpolator()->getX().size() > 2) {
        InterpolatorProcessor::DeleteControlPointsEvent *event = new InterpolatorProcessor::DeleteControlPointsEvent(true, false, false, true);
        envelope->processEvent(event, 0);
        postEvent(event);
    }
}

void EnvelopeClient::postChangeControlPoint(int index, double x, double y)
{
    InterpolatorProcessor::ChangeControlPointEvent *event = new InterpolatorProcessor::ChangeControlPointEvent(index, x, y);
    envelope->processEvent(event, 0);
    postEvent(event);
}

void EnvelopeClient::postChangeSustainIndex(int sustainIndex)
{
    Envelope::ChangeSustainPositionEvent *event = new Envelope::ChangeSustainPositionEvent(sustainIndex);
    envelope->processEvent(event, 0);
    postEvent(event);
}

QGraphicsItem * EnvelopeClient::createGraphicsItem()
{
    return new EnvelopeGraphicsItem(QRectF(0, 0, 1200, 420), this);
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
    // create a child that allows selection of the sustain node:
    sustainNodeItem = new GraphicsMeterItem(QRectF(0, 0, 116, 66), "Sustain node", 1, client->getEnvelope()->getInterpolator()->getX().size() - 1, client->getEnvelope()->getSustainIndex(), client->getEnvelope()->getInterpolator()->getX().size() - 2, 1, GraphicsMeterItem::TOP_HALF, this);
    sustainNodeItem->setBrush(QBrush(Qt::white));
    sustainNodeItem->setPen(QPen(QBrush(Qt::black), 2));
    sustainNodeItem->setFlag(QGraphicsItem::ItemIsMovable);
    QObject::connect(sustainNodeItem, SIGNAL(valueChanged(double)), this, SLOT(onSustainNodeChanged(double)));
}

EnvelopeClient * EnvelopeGraphicsItem::getClient()
{
    return client;
}

void EnvelopeGraphicsItem::increaseControlPoints()
{
    client->postIncreaseControlPoints();
    sustainNodeItem->setRange(1, client->getEnvelope()->getInterpolator()->getX().size() - 1, client->getEnvelope()->getSustainIndex(), client->getEnvelope()->getInterpolator()->getX().size() - 2);
}

void EnvelopeGraphicsItem::decreaseControlPoints()
{
    client->postDecreaseControlPoints();
    sustainNodeItem->setRange(1, client->getEnvelope()->getInterpolator()->getX().size() - 1, client->getEnvelope()->getSustainIndex(), client->getEnvelope()->getInterpolator()->getX().size() - 2);
}

void EnvelopeGraphicsItem::changeControlPoint(int index, double x, double y)
{
    client->postChangeControlPoint(index, x, y);
}

void EnvelopeGraphicsItem::onSustainNodeChanged(double value)
{
    client->postChangeSustainIndex(qRound(value));
    interpolatorChanged();
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
        return new EnvelopeClient(clientName, new Envelope());
    }
    static EnvelopeClientFactory factory;
};

EnvelopeClientFactory EnvelopeClientFactory::factory;

JackClientFactory * EnvelopeClient::getFactory()
{
    return &EnvelopeClientFactory::factory;
}
