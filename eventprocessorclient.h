#ifndef EVENTPROCESSORCLIENT_H
#define EVENTPROCESSORCLIENT_H

#include "eventprocessor.h"
#include "midiprocessorclient.h"
#include "jackringbuffer.h"
#include <jack/jack.h>

template<class T> class EventProcessorClient : public MidiProcessorClient
{
public:
    struct Event {
        jack_nframes_t time;
        T data;
    };

    EventProcessorClient(const QString &clientName, EventProcessor<T> *eventProcessor_, size_t ringBufferSize = 1024) :
            MidiProcessorClient(clientName, eventProcessor_),
            eventProcessor(eventProcessor_),
            eventRingBuffer(ringBufferSize)
    {}
    virtual ~EventProcessorClient()
    {
        close();
    }

protected:
    virtual bool process(jack_nframes_t nframes)
    {
        jack_nframes_t lastFrameTime = getLastFrameTime();
        // get audio port buffers:
        getPortBuffers(nframes);
        // get midi port buffer:
        getMidiPortBuffer(nframes);
        for (jack_nframes_t currentFrame = 0; currentFrame < nframes; ) {
            // get the next event from the ring buffer, if there is any:
            if (eventRingBuffer.readSpace()) {
                Event event = eventRingBuffer.peek();
                // adjust time relative to the beginning of this frame:
                if (event.time + nframes < lastFrameTime) {
                    // if time is too early, this is in the buffer for too long, adjust time accordingly:
                    event.time = 0;
                } else {
                    event.time = event.time + nframes - lastFrameTime;
                }
                if (event.time < nframes) {
                    eventRingBuffer.readAdvance(1);
                    // process everything up to the event's time stamp:
                    processMidi(currentFrame, event.time);
                    currentFrame = event.time;
                    // process the event:
                    processEvent(event.data);
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


    virtual void processEvent(const T &event) = 0;

private:
    EventProcessor<T> *eventProcessor;
    JackRingBuffer<Event> eventRingBuffer;
};

#endif // EVENTPROCESSORCLIENT_H
