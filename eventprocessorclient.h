#ifndef EVENTPROCESSORCLIENT_H
#define EVENTPROCESSORCLIENT_H

#include "midiprocessorclient.h"
#include "jackringbuffer.h"
#include <QVector>

class EventProcessorClient2 : public MidiProcessorClient
{
public:
    EventProcessorClient2(const QString &clientName, MidiProcessor *midiProcessor_, size_t ringBufferSize = (2 << 16));

    bool postEvent(RingBufferEvent *event);
    bool postEvents(const QVector<RingBufferEvent*> &events);

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
    EventProcessorClient2(const QString &clientName, const QStringList &inputPortNames, const QStringList &outputPortNames, size_t ringBufferSize = (2 << 16));

    virtual bool process(jack_nframes_t nframes);
    virtual void processEvent(const RingBufferEvent *event, jack_nframes_t time) = 0;

private:
    RingBuffer ringBuffer;
};

#endif // EVENTPROCESSORCLIENT_H
