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

void IIRMoogFilterClient::setParameters(const IIRMoogFilter::Parameters &parameters)
{
    IIRMoogFilterControl event;
    event.time = getEstimatedCurrentTime();
    event.parameters = parameters;
    controlRingBuffer.write(event);
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
            IIRMoogFilterControl event = controlRingBuffer.peek();
            // adjust time relative to the beginning of this frame:
            if (event.time + nframes < lastFrameTime) {
                // if time is too early, this is in the buffer for too long, adjust time accordingly:
                event.time = 0;
            } else {
                event.time = event.time + nframes - lastFrameTime;
            }
            // test if the event belongs into this frame (and not the next):
            if (event.time < nframes) {
                // do the filtering up to the given time:
                processAudio(currentFrame, event.time);
                currentFrame = event.time;
                // adjust the filter event accordingly:
                filter->setParameters(event.parameters);
                controlRingBuffer.readAdvance(1);
                hasEvents = true;
            }
        }
    }
    // filter till the end of the buffer is reached:
    processAudio(currentFrame, nframes);
    return true;
}
