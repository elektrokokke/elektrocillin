#include "oscillatorclient.h"
#include "graphicsnodeitem.h"

OscillatorClient::OscillatorClient(const QString &clientName, Oscillator *oscillator_, size_t ringBufferSize) :
    EventProcessorClient(clientName, oscillator_, oscillator_, oscillator_, ringBufferSize),
    oscillator(oscillator_)
{
}

OscillatorClient::~OscillatorClient()
{
    close();
    delete oscillator;
}

void OscillatorClient::saveState(QDataStream &stream)
{
    stream << getOscillator()->getGain();
}

void OscillatorClient::loadState(QDataStream &stream)
{
    double gain;
    stream >> gain;
    getOscillator()->setGain(gain);
}

Oscillator * OscillatorClient::getOscillator()
{
    return (Oscillator*)getAudioProcessor();
}

void OscillatorClient::postChangeGain(double gain)
{
    Oscillator::ChangeGainEvent *event = new Oscillator::ChangeGainEvent();
    event->gain = gain;
    postEvent(event);
}

QGraphicsItem * OscillatorClient::createGraphicsItem()
{
    return new OscillatorClientGraphicsItem(QRectF(0, 0, 600, 420), this);
}

bool OscillatorClient::processEvent(const RingBufferEvent *event, jack_nframes_t time)
{
    return oscillator->processEvent(event, time);
}

OscillatorClientGraphicsItem::OscillatorClientGraphicsItem(const QRectF &rect, OscillatorClient *client_, QGraphicsItem *parent, const QPen &nodePen, const QBrush &nodeBrush) :
    QGraphicsRectItem(rect, parent),
    client(client_)
{
    setPen(QPen(QBrush(Qt::black), 2));
    setBrush(Qt::white);
    GraphicsNodeItem *nodeItem = new GraphicsNodeItem(-5.0, -5.0, 10.0, 10.0, this);
    nodeItem->setPen(nodePen);
    nodeItem->setBrush(nodeBrush);
    nodeItem->setZValue(1);
    nodeItem->setBounds(QRectF(rect.center().x(), rect.top(), 0, rect.height()));
    nodeItem->setBoundsScaled(QRectF(0, 1, 0, -1));
    nodeItem->setYScaled(client->getOscillator()->getGain());
    QObject::connect(nodeItem, SIGNAL(yChangedScaled(qreal)), this, SLOT(onNodeYChanged(qreal)));
}

void OscillatorClientGraphicsItem::onNodeYChanged(qreal y)
{
    client->postChangeGain(y);
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
