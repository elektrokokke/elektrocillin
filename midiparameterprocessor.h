#ifndef MIDIPARAMETERPROCESSOR_H
#define MIDIPARAMETERPROCESSOR_H

#include "midiprocessor.h"
#include "parameterprocessor.h"

class MidiParameterProcessor : public MidiProcessor, public ParameterProcessor
{
public:
    MidiParameterProcessor(const QStringList &additionMidiInputPortNames, const QStringList &additionMidiOutputPortNames, MidiProcessor::MidiWriter *midiWriter = 0);

    // reimplemented from MidiProcessor:
    virtual void processController(int inputIndex, unsigned char channel, unsigned char controller, unsigned char value, jack_nframes_t time);
    // reimplemented from ParameterProcessor:
    virtual bool setParameterValue(int index, double value, double min, double max, unsigned int time);
private:
    unsigned char channel;
};

#endif // MIDIPARAMETERPROCESSOR_H
