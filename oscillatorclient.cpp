#include "oscillatorclient.h"

OscillatorClient::OscillatorClient(const QString &clientName, Oscillator *oscillator_, size_t ringBufferSize) :
    EventProcessorClient(clientName, oscillator_, oscillator_, oscillator_, ringBufferSize),
    oscillatorProcess(oscillator_)
{
    oscillator = *oscillatorProcess;
}

OscillatorClient::~OscillatorClient()
{
    close();
    delete oscillatorProcess;
}

void OscillatorClient::saveState(QDataStream &stream)
{
    oscillator.save(stream);
}

void OscillatorClient::loadState(QDataStream &stream)
{
    oscillatorProcess->load(stream);
    oscillator = *oscillatorProcess;
}

Oscillator * OscillatorClient::getOscillator()
{
    return &oscillator;
}

void OscillatorClient::postChangeGain(double gain)
{
    Oscillator::ChangeGainEvent *event = new Oscillator::ChangeGainEvent(gain);
    oscillator.setGain(gain);
    postEvent(event);
}

void OscillatorClient::postChangeTune(double tune)
{
    Oscillator::ChangeTuneEvent *event = new Oscillator::ChangeTuneEvent(tune);
    oscillator.setTune(tune);
    postEvent(event);
}

void OscillatorClient::postChangePitchModulationIntensity(double halfTones)
{
    Oscillator::ChangePitchModulationIntensityEvent *event = new Oscillator::ChangePitchModulationIntensityEvent(halfTones);
    oscillator.setPitchModulationIntensity(halfTones);
    postEvent(event);
}

QGraphicsItem * OscillatorClient::createGraphicsItem()
{
    return new OscillatorClientGraphicsItem(this);
}

OscillatorClientGraphicsItem::OscillatorClientGraphicsItem(OscillatorClient *client_, QGraphicsItem *parent) :
    QGraphicsPathItem(parent),
    client(client_)
{
    GraphicsMeterItem *gainItem = new GraphicsMeterItem(QRectF(0, 0, 116, 66), "Gain", 0, 1, client->getOscillator()->getGain(), 10, 10, GraphicsMeterItem::TOP_HALF, this);
    GraphicsMeterItem *detuneItem = new GraphicsMeterItem(QRectF(0, 66, 116, 66), "Tune", -100, 100, client->getOscillator()->getTune(), 10, 20, GraphicsMeterItem::BOTTOM_HALF, this);
    GraphicsMeterItem *pitchModItem = new GraphicsMeterItem(QRectF(0, 132, 116, 66), "Pitchmod", 0, 12, client->getOscillator()->getPitchModulationIntensity(), 12, 1, GraphicsMeterItem::TOP_HALF, this);
    QObject::connect(gainItem, SIGNAL(valueChanged(double)), this, SLOT(onGainChanged(double)));
    QObject::connect(detuneItem, SIGNAL(valueChanged(double)), this, SLOT(onDetuneChanged(double)));
    QObject::connect(pitchModItem, SIGNAL(valueChanged(double)), this, SLOT(onPitchModulationIntensityChanged(double)));
    QPainterPath path;
    path.addRect(gainItem->boundingRect() | detuneItem->boundingRect() | pitchModItem->boundingRect());
    setPath(path);
    setBrush(QBrush(Qt::white));
}

void OscillatorClientGraphicsItem::onGainChanged(double value)
{
    client->postChangeGain(value);
}

void OscillatorClientGraphicsItem::onDetuneChanged(double value)
{
    client->postChangeTune(value);
}

void OscillatorClientGraphicsItem::onPitchModulationIntensityChanged(double value)
{
    client->postChangePitchModulationIntensity(value);
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
