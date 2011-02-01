#include "iirmoogfilterclient.h"

IIRMoogFilterControlThread::IIRMoogFilterControlThread(JackClientWithDeferredProcessing *client, QObject *parent) :
        JackThread(client, parent),
        ringBufferToClient(1024)
{
}

JackRingBuffer<IIRMoogFilterControl> * IIRMoogFilterControlThread::getOutputRingBuffer()
{
    return &ringBufferToClient;
}

void IIRMoogFilterControlThread::processDeferred()
{
    // this thread does not reveice any messages from the associated Jack client,
    // thus this method does nothing.
}

IIRMoogFilterClient::IIRMoogFilterClient(const QString &clientName) :
        JackClientWithDeferredProcessing(clientName, &thread),
        thread(this),
        filter(440, 0, 1)
{
}

IIRMoogFilterControlThread * IIRMoogFilterClient::getControlThread()
{
    return (IIRMoogFilterControlThread*)getJackThread();
}

JackRingBuffer<IIRMoogFilterControl> * IIRMoogFilterClient::getInputRingBuffer()
{
    return getControlThread()->getOutputRingBuffer();
}

bool IIRMoogFilterClient::init()
{
    // initialize the Moog filter:
    filter.reset();
    filter.setSampleRate(getSampleRate());
    // TODO: initialize the audio buffers
    return true;
}

bool IIRMoogFilterClient::process(jack_nframes_t nframes)
{
    // TODO: get the audio buffers
    for (jack_nframes_t i = 0; i < nframes; i++) {
        // TODO: get the input sample
        float x = 0;
        float y = filter.filter(x);
        // TODO: write the output sample
    }
    return true;
}
