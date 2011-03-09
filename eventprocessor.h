#ifndef EVENTPROCESSOR_H
#define EVENTPROCESSOR_H

#include "midiprocessor.h"
#include "jackringbuffer.h"

class EventProcessor : public MidiProcessor
{
public:
    EventProcessor(const QStringList &inputAudioPortNames, const QStringList &outputAudioPortNames, double sampleRate = 44100);
    EventProcessor(const EventProcessor &tocopy);

    virtual bool processEvent(const RingBufferEvent *event, jack_nframes_t time) = 0;
};

#endif // EVENTPROCESSOR_H
