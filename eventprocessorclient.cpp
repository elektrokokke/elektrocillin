#include "eventprocessorclient.h"

EventProcessorClient::EventProcessorClient(const QString &clientName, MidiProcessor *midiProcessor_, size_t ringBufferSize) :
    MidiProcessorClient(clientName, midiProcessor_),
    ringBuffer(ringBufferSize)
{}

bool EventProcessorClient::postEvent(RingBufferEvent *event)
{
    if (isActive()) {
        jack_nframes_t time = getEstimatedCurrentTime();
        return ringBuffer.sendEvent(event, time);
    } else {
        return false;
    }
}

bool EventProcessorClient::postEvents(const QVector<RingBufferEvent*> &events)
{
    if (isActive()) {
        int writtenEvents = 0;
        jack_nframes_t time = getEstimatedCurrentTime();
        for (int i = 0; i < events.size(); i++) {
            if (ringBuffer.sendEvent(events[i], time)) {
                writtenEvents++;
            }
        }
        return (writtenEvents == events.size());
    } else {
        return false;
    }
}

EventProcessorClient::EventProcessorClient(const QString &clientName, const QStringList &inputPortNames, const QStringList &outputPortNames, size_t ringBufferSize) :
    MidiProcessorClient(clientName, inputPortNames, outputPortNames),
    ringBuffer(ringBufferSize)
{}

bool EventProcessorClient::process(jack_nframes_t nframes)
{
    jack_nframes_t lastFrameTime = getLastFrameTime();
    // get audio port buffers:
    getPortBuffers(nframes);
    // get midi port buffer:
    getMidiPortBuffer(nframes);
    for (jack_nframes_t currentFrame = 0; currentFrame < nframes; ) {
        // get the next event from the ring buffer, if there is any:
        if (ringBuffer.hasEvents()) {
            jack_nframes_t eventTime = ringBuffer.peekEventTime();
            // adjust time relative to the beginning of this frame:
            if (eventTime + nframes < lastFrameTime) {
                // if time is too early, this is in the buffer for too long, adjust time accordingly:
                eventTime = 0;
            } else {
                eventTime = eventTime + nframes - lastFrameTime;
            }
            if (eventTime < nframes) {
                // process everything up to the event's time stamp:
                processMidi(currentFrame, eventTime);
                currentFrame = eventTime;
                // process the event:
                RingBufferEvent *event = ringBuffer.readEvent();
                processEvent(event, eventTime);
                // have the event deleted in the creator thread:
                ringBuffer.returnEvent(event);
            } else {
                processMidi(currentFrame, nframes);
                currentFrame = nframes;
            }
        } else {
            processMidi(currentFrame, nframes);
            currentFrame = nframes;
        }
    }
    return true;
}
