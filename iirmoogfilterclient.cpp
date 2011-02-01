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

void IIRMoogFilterControlThread::setParameters(double cutoffFrequency, double resonance)
{
    IIRMoogFilterControl parameters;
    parameters.time = getClient()->getEstimatedCurrentTime();
    parameters.cutoffFrequency = cutoffFrequency;
    parameters.resonance = resonance;
    getOutputRingBuffer()->write(parameters);
}

void IIRMoogFilterControlThread::processDeferred()
{
    // this thread does not reveice any messages from the associated Jack client,
    // thus this method does nothing.
}

IIRMoogFilterClient::IIRMoogFilterClient(const QString &clientName) :
    JackClientWithDeferredProcessing(clientName, &thread),
    thread(this),
    filter(440, 0, 1),
    audioInputPortName("audio in"),
    audioOutputPortName("audio out")
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
    // initialize the audio buffers:
    audioInputPort = registerAudioPort(audioInputPortName, JackPortIsInput);
    audioOutputPort = registerAudioPort(audioOutputPortName, JackPortIsOutput);
    return audioInputPort && audioOutputPort;
}

bool IIRMoogFilterClient::process(jack_nframes_t nframes)
{
    // get port buffers:
    const jack_default_audio_sample_t *audioInputBuffer = reinterpret_cast<jack_default_audio_sample_t*>(jack_port_get_buffer(audioInputPort, nframes));
    jack_default_audio_sample_t *audioOutputBuffer = reinterpret_cast<jack_default_audio_sample_t*>(jack_port_get_buffer(audioOutputPort, nframes));

    jack_nframes_t lastFrameTime = getLastFrameTime();
    jack_nframes_t currentFrame = 0;
    for (bool hasEvents = true; hasEvents; ) {
        hasEvents = false;
        if (getInputRingBuffer()->readSpace()) {
            // get the message from the ring buffer:
            IIRMoogFilterControl parameters = getInputRingBuffer()->peek();
            // adjust time relative to the beginning of this frame:
            if (parameters.time + nframes < lastFrameTime) {
                // if time is too early, this is in the buffer for too long, adjust time accordingly:
                parameters.time = 0;
            } else {
                parameters.time = parameters.time + nframes - lastFrameTime;
            }
            // test if the event belongs into this frame (and not the next):
            if (parameters.time < nframes) {
                // do the filtering up to the given time:
                for (; currentFrame < parameters.time; currentFrame++) {
                    audioOutputBuffer[currentFrame] = filter.filter(audioInputBuffer[currentFrame]);
                }
                // adjust the filter parameters accordingly:
                filter.setCutoffFrequency(parameters.cutoffFrequency, parameters.resonance);
                getInputRingBuffer()->readAdvance(1);
                hasEvents = true;
            }
        }
    }
    // filter till the end of the buffer is reached:
    for (; currentFrame < nframes; currentFrame++) {
        audioOutputBuffer[currentFrame] = filter.filter(audioInputBuffer[currentFrame]);
    }
    return true;
}
