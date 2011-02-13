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
    JackThreadEventProcessorClient<IirMoogFilter::Parameters>(new IirMoogFilterThread(this), clientName, filter, ringBufferSize),
    ringBufferToThread(ringBufferSize)
{
    getMoogFilterThread()->setRingBufferFromClient(&ringBufferToThread);
}

IirMoogFilterClient::~IirMoogFilterClient()
{
    close();
    delete getMoogFilterThread();
}

IirMoogFilter * IirMoogFilterClient::getMoogFilter()
{
    return (IirMoogFilter*)getMidiProcessor();
}

IirMoogFilterThread * IirMoogFilterClient::getMoogFilterThread()
{
    return (IirMoogFilterThread*)getJackThread();
}

void IirMoogFilterClient::processEvent(const IirMoogFilter::Parameters &event, jack_nframes_t)
{
    getMoogFilter()->setParameters(event);
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
