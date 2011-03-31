#include "oscillatorclient.h"
#include "graphicsdiscretecontrolitem.h"
#include <QPen>
#include <QBrush>

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
    EventProcessorClient::saveState(stream);
    oscillator.save(stream);
}

void OscillatorClient::loadState(QDataStream &stream)
{
    EventProcessorClient::loadState(stream);
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
    QGraphicsRectItem(parent),
    client(client_)
{
    setFlags(QGraphicsItem::ItemIsFocusable);
    GraphicsContinuousControlItem *gainControl = new GraphicsContinuousControlItem("Gain", 0, 1, client->getOscillator()->getGain(), 250, GraphicsContinuousControlItem::HORIZONTAL, 'g', -1, 0.01, this);
    GraphicsContinuousControlItem *tuneControl = new GraphicsContinuousControlItem("Tune", -100, 100, client->getOscillator()->getTune(), 500, GraphicsContinuousControlItem::HORIZONTAL, 'g', -1, 1, this);
    GraphicsContinuousControlItem *pitchModControl = new GraphicsContinuousControlItem("Pitch mod. intensity", 0, 24, client->getOscillator()->getPitchModulationIntensity(), 240, GraphicsContinuousControlItem::HORIZONTAL, 'g', -1, 1, this);
    gainControl->setPos(4, 4 + (4 + gainControl->boundingRect().height()) * 0);
    tuneControl->setPos(4, 4 + (4 + gainControl->boundingRect().height()) * 1);
    pitchModControl->setPos(4, 4 + (4 + gainControl->boundingRect().height()) * 2);
    QObject::connect(gainControl, SIGNAL(valueChanged(double)), this, SLOT(onGainChanged(double)));
    QObject::connect(tuneControl, SIGNAL(valueChanged(double)), this, SLOT(onDetuneChanged(double)));
    QObject::connect(pitchModControl, SIGNAL(valueChanged(double)), this, SLOT(onPitchModulationIntensityChanged(double)));
    setRect((gainControl->rect().translated(gainControl->pos()) | tuneControl->rect().translated(tuneControl->pos()) | pitchModControl->rect().translated(pitchModControl->pos())).adjusted(-4, -4, 4, 4));
    setPen(QPen(QBrush(Qt::black), 1));
    setBrush(QBrush(Qt::white));
}

void OscillatorClientGraphicsItem::focusInEvent(QFocusEvent * event)
{
    setZValue(1);
}

void OscillatorClientGraphicsItem::focusOutEvent(QFocusEvent * event)
{
    setZValue(0);
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
