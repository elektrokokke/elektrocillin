#ifndef EVENTPROCESSORCLIENT_H
#define EVENTPROCESSORCLIENT_H

#include "midiprocessorclient.h"
#include "eventprocessor.h"
#include <QVector>

class EventProcessorClient : public MidiProcessorClient
{
public:
    EventProcessorClient(const QString &clientName, AudioProcessor *audioProcessor, MidiProcessor *midiProcessor, EventProcessor *eventProcessor_, size_t ringBufferSize = (2 << 16));

    EventProcessor * getEventProcessor();

    bool postEvent(RingBufferEvent *event);
    bool postEvents(const QVector<RingBufferEvent*> &events);

protected:
    /**
      Constructor for subclasses that do not want to use an EventProcessor,
      but reimplement the respective methods such as to do the EventProcessor's
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
    EventProcessorClient(const QString &clientName, const QStringList &inputPortNames, const QStringList &outputPortNames, size_t ringBufferSize = (2 << 16));

    // reimplemented from MidiProcessorClient:
    virtual bool process(jack_nframes_t nframes);

    virtual bool processEvent(const RingBufferEvent *event, jack_nframes_t time);

private:
    EventProcessor *eventProcessor;
    RingBuffer ringBuffer;
};

#endif // EVENTPROCESSORCLIENT_H
