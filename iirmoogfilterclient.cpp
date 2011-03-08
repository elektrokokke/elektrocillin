#include "iirmoogfilterclient.h"

IirMoogFilterThread::IirMoogFilterThread(IirMoogFilterClient *client, QObject *parent) :
    JackThread(client, parent),
    ringBufferFromClient(0)
{
}

void IirMoogFilterThread::setRingBufferFromClient(JackRingBuffer<IirMoogFilter::Parameters> *ringBufferFromClient)
{
    this->ringBufferFromClient = ringBufferFromClient;
}

void IirMoogFilterThread::processDeferred()
{
    if (ringBufferFromClient && ringBufferFromClient->readSpace()) {
        ringBufferFromClient->readAdvance(ringBufferFromClient->readSpace() - 1);
        IirMoogFilter::Parameters parameters = ringBufferFromClient->read();
        changedParameters(parameters.frequency, parameters.resonance);
    }
}

IirMoogFilterClient::IirMoogFilterClient(const QString &clientName, size_t ringBufferSize) :
    JackThreadEventProcessorClient(new IirMoogFilterThread(this), clientName, new IirMoogFilter(44100, 1), ringBufferSize),
    ringBufferToThread(ringBufferSize)
{
    getMoogFilterThread()->setRingBufferFromClient(&ringBufferToThread);
}

IirMoogFilterClient::~IirMoogFilterClient()
{
    close();
    delete getMoogFilterThread();
    delete getMoogFilter();
}

void IirMoogFilterClient::saveState(QDataStream &stream)
{
    IirMoogFilter::Parameters parameters = getMoogFilter()->getParameters();
    stream << parameters.frequency;
    stream << parameters.frequencyOffsetFactor;
    stream << parameters.frequencyPitchBendFactor;
    stream << parameters.frequencyModulationFactor;
    stream << parameters.frequencyModulationIntensity;
    stream << parameters.resonance;
}

void IirMoogFilterClient::loadState(QDataStream &stream)
{
    IirMoogFilter::Parameters parameters;
    stream >> parameters.frequency;
    stream >> parameters.frequencyOffsetFactor;
    stream >> parameters.frequencyPitchBendFactor;
    stream >> parameters.frequencyModulationFactor;
    stream >> parameters.frequencyModulationIntensity;
    stream >> parameters.resonance;
    getMoogFilter()->processEvent(&parameters);
}

IirMoogFilter * IirMoogFilterClient::getMoogFilter()
{
    return (IirMoogFilter*)getMidiProcessor();
}

IirMoogFilterThread * IirMoogFilterClient::getMoogFilterThread()
{
    return (IirMoogFilterThread*)getJackThread();
}

QGraphicsItem * IirMoogFilterClient::createGraphicsItem()
{
    return new IirMoogFilterGraphicsItem(this, QRectF(0, 0, 600, 420));
}

bool IirMoogFilterClient::processEvent(const RingBufferEvent *event, jack_nframes_t)
{
    if (const IirMoogFilter::Parameters *parameters = dynamic_cast<const IirMoogFilter::Parameters*>(event)) {
        getMoogFilter()->processEvent(parameters);
        return true;
    }
    return false;
}

void IirMoogFilterClient::processNoteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time)
{
    // call the midi processor's method:
    getMoogFilter()->processNoteOn(channel, noteNumber, velocity, time);
    // notify the associated thread:
    ringBufferToThread.write(getMoogFilter()->getParameters());
    wakeJackThread();
}

void IirMoogFilterClient::processController(unsigned char channel, unsigned char controller, unsigned char value, jack_nframes_t time)
{
    getMoogFilter()->processController(channel, controller, value, time);
    // notify the associated thread:
    ringBufferToThread.write(getMoogFilter()->getParameters());
    wakeJackThread();
}

IirMoogFilterGraphicsItem::IirMoogFilterGraphicsItem(IirMoogFilterClient *client_, const QRectF &rect, QGraphicsItem *parent) :
    FrequencyResponseGraphicsItem(rect, 22050.0 / 512.0, 22050, -30, 30, parent),
    client(client_),
    filterCopy(*client->getMoogFilter())
{
    addFrequencyResponse(&filterCopy);
    cutoffResonanceNode = new GraphicsNodeItem(-5.0, -5.0, 10.0, 10.0, this);
    cutoffResonanceNode->setScale(GraphicsNodeItem::LOGARITHMIC, GraphicsNodeItem::LINEAR);
    cutoffResonanceNode->setPen(QPen(QBrush(qRgb(114, 159, 207)), 3));
    cutoffResonanceNode->setBrush(QBrush(qRgb(52, 101, 164)));
    cutoffResonanceNode->setZValue(10);
    cutoffResonanceNode->setBounds(QRectF(getFrequencyResponseRectangle().topLeft(), QPointF(getFrequencyResponseRectangle().right(), getZeroDecibelY())));
    cutoffResonanceNode->setBoundsScaled(QRectF(QPointF(getLowestHertz(), 1), QPointF(getHighestHertz(), 0)));
    onClientChangedFilterParameters(filterCopy.getParameters().frequency, filterCopy.getParameters().resonance);
    QObject::connect(cutoffResonanceNode, SIGNAL(positionChangedScaled(QPointF)), this, SLOT(onGuiChangedFilterParameters(QPointF)));
    QObject::connect(client->getMoogFilterThread(), SIGNAL(changedParameters(double, double)), this, SLOT(onClientChangedFilterParameters(double,double)));
}

void IirMoogFilterGraphicsItem::onGuiChangedFilterParameters(const QPointF &cutoffResonance)
{
    IirMoogFilter::Parameters *parameters = new IirMoogFilter::Parameters();
    *parameters = filterCopy.getParameters();
    parameters->frequency = cutoffResonance.x();
    parameters->resonance = cutoffResonance.y();
    filterCopy.processEvent(parameters);
    client->postEvent(parameters);
    updateFrequencyResponse(0);
}

void IirMoogFilterGraphicsItem::onClientChangedFilterParameters(double frequency, double resonance)
{
    IirMoogFilter::Parameters parameters = filterCopy.getParameters();
    parameters.frequency = frequency;
    parameters.resonance = resonance;
    filterCopy.processEvent(&parameters);
    cutoffResonanceNode->setXScaled(parameters.frequency);
    cutoffResonanceNode->setYScaled(parameters.resonance);
    updateFrequencyResponse(0);
}

class IirMoogFilterClientFactory : public JackClientFactory
{
public:
    IirMoogFilterClientFactory()
    {
        JackClientSerializer::getInstance()->registerFactory(this);
    }
    QString getName()
    {
        return "Lowpass filter";
    }
    JackClient * createClient(const QString &clientName)
    {
        return new IirMoogFilterClient(clientName);
    }
    static IirMoogFilterClientFactory factory;
};

IirMoogFilterClientFactory IirMoogFilterClientFactory::factory;

JackClientFactory * IirMoogFilterClient::getFactory()
{
    return &IirMoogFilterClientFactory::factory;
}
