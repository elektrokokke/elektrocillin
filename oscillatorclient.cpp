#include "oscillatorclient.h"

OscillatorClient::OscillatorClient(const QString &clientName, Oscillator *oscillator_, size_t ringBufferSize) :
    EventProcessorClient(clientName, oscillator_, oscillator_, oscillator_, ringBufferSize),
    oscillatorProcess(oscillator_),
    gain(oscillatorProcess->getGain())
{
}

OscillatorClient::~OscillatorClient()
{
    close();
    delete oscillatorProcess;
}

void OscillatorClient::saveState(QDataStream &stream)
{
    stream << gain;
}

void OscillatorClient::loadState(QDataStream &stream)
{
    stream >> gain;
    oscillatorProcess->setGain(gain);
}

double OscillatorClient::getGain() const
{
    return gain;
}

void OscillatorClient::postChangeGain(double gain)
{
    Oscillator::ChangeGainEvent *event = new Oscillator::ChangeGainEvent();
    event->gain = gain;
    this->gain = gain;
    postEvent(event);
}

QGraphicsItem * OscillatorClient::createGraphicsItem()
{
    return new OscillatorClientGraphicsItem(this);
}

OscillatorClientGraphicsItem::OscillatorClientGraphicsItem(OscillatorClient *client_, QGraphicsItem *parent, const QPen &nodePen, const QBrush &nodeBrush) :
    QGraphicsPathItem(parent),
    client(client_)
{
    GraphicsMeterItem *gainItem = new GraphicsMeterItem(QRectF(0, 0, 100, 50), "Gain", 0, 1, client->getGain(), 10, this);
    GraphicsMeterItem *detuneItem = new GraphicsMeterItem(QRectF(0, 50, 100, 50), "Tune", -100, 100, 0, 10, this);
    QObject::connect(gainItem, SIGNAL(valueChanged(double)), this, SLOT(onGainChanged(double)));
    QObject::connect(detuneItem, SIGNAL(valueChanged(double)), this, SLOT(onDetuneChanged(double)));
    setPath(gainItem->shape() + detuneItem->shape());
    setPen(QPen(Qt::NoPen));
}

void OscillatorClientGraphicsItem::onGainChanged(double value)
{
    client->postChangeGain(value);
}

void OscillatorClientGraphicsItem::onDetuneChanged(double value)
{
    // TODO: post detune change
}

class OscillatorClientFactory : public JackClientFactory
{
public:
    OscillatorClientFactory()
    {
        JackClientSerializer::getInstance()->registerFactory(this);
    }
    QString getName()
    {
        return "Sine oscillator";
    }
    JackClient * createClient(const QString &clientName)
    {
        return new OscillatorClient(clientName, new Oscillator());
    }
    static OscillatorClientFactory factory;
};

OscillatorClientFactory OscillatorClientFactory::factory;

JackClientFactory * OscillatorClient::getFactory()
{
    return &OscillatorClientFactory::factory;
}
