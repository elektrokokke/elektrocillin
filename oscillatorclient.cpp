#include "oscillatorclient.h"

OscillatorClient::OscillatorClient(const QString &clientName, Oscillator *oscillator_, size_t ringBufferSize) :
    EventProcessorClient(clientName, oscillator_, oscillator_, oscillator_, ringBufferSize),
    oscillatorProcess(oscillator_),
    gain(oscillatorProcess->getGain()),
    tune(oscillatorProcess->getTune()),
    pitchModulationIntensity(oscillatorProcess->getPitchModulationIntensity())
{
}

OscillatorClient::~OscillatorClient()
{
    close();
    delete oscillatorProcess;
}

void OscillatorClient::saveState(QDataStream &stream)
{
    stream << gain << tune << pitchModulationIntensity;
}

void OscillatorClient::loadState(QDataStream &stream)
{
    stream >> gain >> tune >> pitchModulationIntensity;
    oscillatorProcess->setGain(gain);
    oscillatorProcess->setTune(tune);
    oscillatorProcess->setPitchModulationIntensity(pitchModulationIntensity);
}

double OscillatorClient::getGain() const
{
    return gain;
}

void OscillatorClient::postChangeGain(double gain)
{
    Oscillator::ChangeGainEvent *event = new Oscillator::ChangeGainEvent(gain);
    this->gain = gain;
    postEvent(event);
}

double OscillatorClient::getTune() const
{
    return tune;
}

void OscillatorClient::postChangeTune(double tune)
{
    Oscillator::ChangeTuneEvent *event = new Oscillator::ChangeTuneEvent(tune);
    this->tune = tune;
    postEvent(event);
}

double OscillatorClient::getPitchModulationIntensity() const
{
    return pitchModulationIntensity;
}

void OscillatorClient::postChangePitchModulationIntensity(double halfTones)
{
    Oscillator::ChangePitchModulationIntensityEvent *event = new Oscillator::ChangePitchModulationIntensityEvent(halfTones);
    this->pitchModulationIntensity = halfTones;
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
    GraphicsMeterItem *gainItem = new GraphicsMeterItem(QRectF(0, 0, 116, 66), "Gain", 0, 1, client->getGain(), 10, 10, GraphicsMeterItem::TOP_HALF, this);
    GraphicsMeterItem *detuneItem = new GraphicsMeterItem(QRectF(0, 66, 116, 66), "Tune", -100, 100, client->getTune(), 10, 20, GraphicsMeterItem::BOTTOM_HALF, this);
    GraphicsMeterItem *pitchModItem = new GraphicsMeterItem(QRectF(0, 132, 116, 66), "Pitchmod", 0, 12, client->getPitchModulationIntensity(), 12, 1, GraphicsMeterItem::TOP_HALF, this);
    QObject::connect(gainItem, SIGNAL(valueChanged(double)), this, SLOT(onGainChanged(double)));
    QObject::connect(detuneItem, SIGNAL(valueChanged(double)), this, SLOT(onDetuneChanged(double)));
    QObject::connect(pitchModItem, SIGNAL(valueChanged(double)), this, SLOT(onPitchModulationIntensityChanged(double)));
    QPainterPath path;
    path.addRect(gainItem->boundingRect() | detuneItem->boundingRect() | pitchModItem->boundingRect());
    setPath(path);
    setPen(QPen(Qt::NoPen));
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
