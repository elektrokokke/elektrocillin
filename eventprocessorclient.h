#ifndef EVENTPROCESSORCLIENT_H
#define EVENTPROCESSORCLIENT_H

#include "midiprocessorclient.h"
#include "jackringbuffer.h"
#include <QVector>

template<class T> class EventProcessorClient : public MidiProcessorClient
{
public:
    struct EventWithTimeStamp {
        jack_nframes_t time;
        T event;
    };

    EventProcessorClient(const QString &clientName, MidiProcessor *midiProcessor_, size_t ringBufferSize = 1024) :
        MidiProcessorClient(clientName, midiProcessor_),
        eventRingBuffer(ringBufferSize)
    {}

    void postEvent(const T &event)
    {
        if (isActive()) {
            EventWithTimeStamp ev;
            ev.time = getEstimatedCurrentTime();
            ev.event = event;
            eventRingBuffer.write(ev);
        }
    }

    void postEvents(const QVector<T> &events)
    {
        if (isActive()) {
            jack_nframes_t time = getEstimatedCurrentTime();
            for (int i = 0; i < events.size(); i++) {
                EventWithTimeStamp ev;
                ev.time = time;
                ev.event = events[i];
                eventRingBuffer.write(ev);
            }
        }
    }

protected:
    /**
      Constructor for subclasses that do not want to use a MidiProcessor,
      but reimplement the respective methods such as to do the MidiProcessor's
      work themselves.

      See AudioProcessorClient::AudioProcessorClient(const QString &, const QStringList &, const QStringList &)
      for a parameter description.

      Methods to reimplement:
      - processAudio(const double*, double*, jack_nframes_t) OR
      - processAudio(jack_nframes_t, jack_nframes_t),
      - processNoteOn AND
      - processNoteOff AND
      - processController AND
      - processPitchBend OR
      - processMidi(const MidiEvent &, jack_nframes_t) OR
      - processMidi(jack_nframes_t, jack_nframes_t)
      */
    EventProcessorClient(const QString &clientName, const QStringList &inputPortNames, const QStringList &outputPortNames, size_t ringBufferSize = 1024) :
        MidiProcessorClient(clientName, inputPortNames, outputPortNames),
        eventRingBuffer(ringBufferSize)
    {}

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
                EventWithTimeStamp eventWithTimeStamp = eventRingBuffer.peek();
                // adjust time relative to the beginning of this frame:
                if (eventWithTimeStamp.time + nframes < lastFrameTime) {
                    // if time is too early, this is in the buffer for too long, adjust time accordingly:
                    eventWithTimeStamp.time = 0;
                } else {
                    eventWithTimeStamp.time = eventWithTimeStamp.time + nframes - lastFrameTime;
                }
                if (eventWithTimeStamp.time < nframes) {
                    eventRingBuffer.readAdvance(1);
                    // process everything up to the event's time stamp:
                    processMidi(currentFrame, eventWithTimeStamp.time);
                    currentFrame = eventWithTimeStamp.time;
                    // process the event:
                    processEvent(eventWithTimeStamp.event, eventWithTimeStamp.time);
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


    virtual void processEvent(const T &event, jack_nframes_t time) = 0;

private:
    JackRingBuffer<EventWithTimeStamp> eventRingBuffer;
};

class EventProcessorClient2 : public MidiProcessorClient
{
public:
    EventProcessorClient2(const QString &clientName, MidiProcessor *midiProcessor_, size_t ringBufferSize = (2 << 16)) :
        MidiProcessorClient(clientName, midiProcessor_),
        ringBuffer(ringBufferSize)
    {}

    bool postEvent(const RingBufferEvent *event)
    {
        if (isActive()) {
            jack_nframes_t time = getEstimatedCurrentTime();
            return ringBuffer.write(event, time);
        } else {
            return false;
        }
    }

    bool postEvents(const QVector<const RingBufferEvent*> &events)
    {
        if (isActive()) {
            int writtenEvents = 0;
            jack_nframes_t time = getEstimatedCurrentTime();
            for (int i = 0; i < events.size(); i++) {
                if (ringBuffer.write(events[i], time)) {
                    writtenEvents++;
                }
            }
            return (writtenEvents == events.size());
        } else {
            return false;
        }
    }

protected:
    /**
      Constructor for subclasses that do not want to use a MidiProcessor,
      but reimplement the respective methods such as to do the MidiProcessor's
      work themselves.

      See AudioProcessorClient::AudioProcessorClient(const QString &, const QStringList &, const QStringList &)
      for a parameter description.

      Methods to reimplement:
      - processAudio(const double*, double*, jack_nframes_t) OR
      - processAudio(jack_nframes_t, jack_nframes_t),
      - processNoteOn AND
      - processNoteOff AND
      - processController AND
      - processPitchBend OR
      - processMidi(const MidiEvent &, jack_nframes_t) OR
      - processMidi(jack_nframes_t, jack_nframes_t)
      */
    EventProcessorClient2(const QString &clientName, const QStringList &inputPortNames, const QStringList &outputPortNames, size_t ringBufferSize = (2 << 16)) :
        MidiProcessorClient(clientName, inputPortNames, outputPortNames),
        ringBuffer(ringBufferSize)
    {}

    virtual bool process(jack_nframes_t nframes)
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
                    RingBufferEvent *event = ringBuffer.read();
                    processEvent(event, eventTime);
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

    virtual void processEvent(const RingBufferEvent *event, jack_nframes_t time) = 0;

private:
    RingBuffer ringBuffer;
};

#endif // EVENTPROCESSORCLIENT_H
