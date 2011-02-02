#include "iirmoogfilterclient.h"

IIRMoogFilterClient::IIRMoogFilterClient(const QString &clientName, QObject *parent) :
    QObject(parent),
    JackClient(clientName),
    controlRingBuffer(1024),
    filter(440, 0, 1, 1),
    audioInputPortName("audio in"),
    audioOutputPortName("audio out")
{
}

IIRMoogFilterClient::~IIRMoogFilterClient()
{
    close();
}

void IIRMoogFilterClient::setParameters(double cutoffFrequency, double resonance)
{
    IIRMoogFilterControl parameters;
    parameters.time = getEstimatedCurrentTime();
    parameters.cutoffFrequency = cutoffFrequency;
    parameters.resonance = resonance;
    controlRingBuffer.write(parameters);
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
        if (controlRingBuffer.readSpace()) {
            // get the message from the ring buffer:
            IIRMoogFilterControl parameters = controlRingBuffer.peek();
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
                    audioOutputBuffer[currentFrame] = filter.process1(audioInputBuffer[currentFrame]);
                }
                // adjust the filter parameters accordingly:
                filter.setCutoffFrequency(parameters.cutoffFrequency, parameters.resonance);
                controlRingBuffer.readAdvance(1);
                hasEvents = true;
            }
        }
    }
    // filter till the end of the buffer is reached:
    for (; currentFrame < nframes; currentFrame++) {
        audioOutputBuffer[currentFrame] = filter.process1(audioInputBuffer[currentFrame]);
    }
    return true;
}
