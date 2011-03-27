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

IirMoogFilterClient::IirMoogFilterClient(const QString &clientName, IirMoogFilter *filter, size_t ringBufferSize) :
    JackThreadEventProcessorClient(new IirMoogFilterThread(this), clientName, filter, filter, filter, ringBufferSize),
    iirMoogFilterProcess(filter),
    ringBufferToThread(ringBufferSize)
{
    iirMoogFilter = new IirMoogFilter(44100, 1);
    iirMoogFilter->setParameters(&iirMoogFilterProcess->getParameters());
    getMoogFilterThread()->setRingBufferFromClient(&ringBufferToThread);
    QObject::connect(getMoogFilterThread(), SIGNAL(changedParameters(double, double)), this, SLOT(onClientChangedFilterParameters(double,double)));
}

IirMoogFilterClient::~IirMoogFilterClient()
{
    // calling close will stop the Jack client and also stop the associated IirMoogFilterThread:
    close();
    // deleting the filter is now safe, as it is not used anymore (the Jack process thread is stopped):
    delete iirMoogFilterProcess;
    delete iirMoogFilter;
    // deleting the associated thread is now safe, as it has been stopped:
    delete getJackThread();
}

void IirMoogFilterClient::saveState(QDataStream &stream)
{
    IirMoogFilter::Parameters parameters = iirMoogFilter->getParameters();
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
    iirMoogFilter->setParameters(&parameters);
    iirMoogFilterProcess->setParameters(&parameters);
}

IirMoogFilter * IirMoogFilterClient::getMoogFilter()
{
    return iirMoogFilter;
}

IirMoogFilterThread * IirMoogFilterClient::getMoogFilterThread()
{
    return (IirMoogFilterThread*)getJackThread();
}

QGraphicsItem * IirMoogFilterClient::createGraphicsItem()
{
    return new IirMoogFilterGraphicsItem(this, QRectF(0, 0, 600, 420));
}

void IirMoogFilterClient::processNoteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time)
{
    // call base implementation:
    MidiProcessorClient::processNoteOn(channel, noteNumber, velocity, time);
    // notify the associated thread:
    ringBufferToThread.write(iirMoogFilterProcess->getParameters());
    wakeJackThread();
}

void IirMoogFilterClient::processController(unsigned char channel, unsigned char controller, unsigned char value, jack_nframes_t time)
{
    // call base implementation:
    MidiProcessorClient::processController(channel, controller, value, time);
    // notify the associated thread:
    ringBufferToThread.write(iirMoogFilterProcess->getParameters());
    wakeJackThread();
}

void IirMoogFilterClient::onClientChangedFilterParameters(double frequency, double resonance)
{
    IirMoogFilter::Parameters parameters = iirMoogFilter->getParameters();
    parameters.frequency = frequency;
    parameters.resonance = resonance;
    iirMoogFilter->setParameters(&parameters);
}

IirMoogFilterGraphicsItem::IirMoogFilterGraphicsItem(IirMoogFilterClient *client_, const QRectF &rect, QGraphicsItem *parent) :
    FrequencyResponseGraphicsItem(rect, 22050.0 / 512.0, 22050, -30, 30, parent),
    client(client_)
{
    addFrequencyResponse(client->getMoogFilter());
    cutoffResonanceNode = new GraphicsNodeItem(-5.0, -5.0, 10.0, 10.0, this);
    cutoffResonanceNode->setScale(GraphicsNodeItem::LOGARITHMIC, GraphicsNodeItem::LINEAR);
    cutoffResonanceNode->setPen(QPen(QBrush(qRgb(114, 159, 207)), 3));
    cutoffResonanceNode->setBrush(QBrush(qRgb(52, 101, 164)));
    cutoffResonanceNode->setZValue(10);
    cutoffResonanceNode->setBounds(QRectF(getFrequencyResponseRectangle().topLeft(), QPointF(getFrequencyResponseRectangle().right(), getZeroDecibelY())));
    cutoffResonanceNode->setBoundsScaled(QRectF(QPointF(getLowestHertz(), 1), QPointF(getHighestHertz(), 0)));
    IirMoogFilter::Parameters parameters = client->getMoogFilter()->getParameters();
    onClientChangedFilterParameters(parameters.frequency, parameters.resonance);
    QObject::connect(cutoffResonanceNode, SIGNAL(positionChangedScaled(QPointF)), this, SLOT(onGuiChangedFilterParameters(QPointF)));
    QObject::connect(client->getMoogFilterThread(), SIGNAL(changedParameters(double, double)), this, SLOT(onClientChangedFilterParameters(double,double)));
}

void IirMoogFilterGraphicsItem::onGuiChangedFilterParameters(const QPointF &cutoffResonance)
{
    IirMoogFilter::Parameters *parameters = new IirMoogFilter::Parameters();
    *parameters = client->getMoogFilter()->getParameters();
    parameters->frequency = cutoffResonance.x();
    parameters->resonance = cutoffResonance.y();
    client->getMoogFilter()->setParameters(parameters);
    client->postEvent(parameters);
    updateFrequencyResponse(0);
}

void IirMoogFilterGraphicsItem::onClientChangedFilterParameters(double frequency, double resonance)
{
    cutoffResonanceNode->setXScaled(frequency);
    cutoffResonanceNode->setYScaled(resonance);
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
        return new IirMoogFilterClient(clientName, new IirMoogFilter(44100, 1));
    }
    static IirMoogFilterClientFactory factory;
};

IirMoogFilterClientFactory IirMoogFilterClientFactory::factory;

JackClientFactory * IirMoogFilterClient::getFactory()
{
    return &IirMoogFilterClientFactory::factory;
}
