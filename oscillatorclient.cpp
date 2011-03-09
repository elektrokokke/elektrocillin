#include "oscillatorclient.h"
#include "graphicsnodeitem.h"

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
    return new OscillatorClientGraphicsItem(QRectF(0, 0, 600, 420), this);
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
    nodeItem->setYScaled(client->getGain());
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
