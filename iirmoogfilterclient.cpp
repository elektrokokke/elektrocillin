#include "iirmoogfilterclient.h"

IIRMoogFilterClient::IIRMoogFilterClient(const QString &clientName, IIRMoogFilter *filter_) :
    AudioProcessorClient(clientName, filter_),
    filter(filter_),
    controlRingBuffer(1024)
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
    filter->reset();
    filter->setSampleRate(getSampleRate());
    return AudioProcessorClient::init();
}

bool IIRMoogFilterClient::process(jack_nframes_t nframes)
{
    getPortBuffers(nframes);
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
                processAudio(currentFrame, parameters.time);
                currentFrame = parameters.time;
                // adjust the filter parameters accordingly:
                filter->setCutoffFrequency(parameters.cutoffFrequency, parameters.resonance);
                controlRingBuffer.readAdvance(1);
                hasEvents = true;
            }
        }
    }
    // filter till the end of the buffer is reached:
    processAudio(currentFrame, nframes);
    return true;
}
