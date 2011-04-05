#ifndef MIDIPARAMETERPROCESSOR_H
#define MIDIPARAMETERPROCESSOR_H

#include "midiprocessor.h"
#include "parameterprocessor.h"

class MidiParameterProcessor : public MidiProcessor, public ParameterProcessor
{
public:
    MidiParameterProcessor();

    // reimplemented from MidiProcessor:
    virtual void processController(unsigned char channel, unsigned char controller, unsigned char value, jack_nframes_t time);
    // reimplemented from ParameterProcessor:
    virtual bool setParameterValue(int index, double value, unsigned int time);
private:
    unsigned char channel;
};

#endif // MIDIPARAMETERPROCESSOR_H
