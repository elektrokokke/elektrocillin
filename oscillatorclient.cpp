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
    return new OscillatorClientGraphicsItem(QRectF(0, 0, 100, 50), this);
}

OscillatorClientGraphicsItem::OscillatorClientGraphicsItem(const QRectF &rect, OscillatorClient *client_, QGraphicsItem *parent, const QPen &nodePen, const QBrush &nodeBrush) :
    GraphicsMeterItem(rect, "Gain", 0, 1, client_->getGain(), 10, parent),
    client(client_)
{
    QObject::connect(this, SIGNAL(valueChanged(double)), this, SLOT(onValueChanged(double)));
}

void OscillatorClientGraphicsItem::onValueChanged(double value)
{
    client->postChangeGain(value);
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
