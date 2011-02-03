#ifndef EVENTPROCESSOR_H
#define EVENTPROCESSOR_H

#include "midiprocessor.h"
#include "jackringbuffer.h"
#include <jack/jack.h>

template<class T> class EventProcessor : public MidiProcessor
{
public:
    EventProcessor(int nrOfInputs, int nrOfOutputs, double sampleRate = 44100) :
            MidiProcessor(nrOfInputs, nrOfOutputs, sampleRate)
    {}

    virtual void processEvent(const T &event);
};

#endif // EVENTPROCESSOR_H
