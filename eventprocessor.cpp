#include "eventprocessor.h"

EventProcessor::EventProcessor(const QStringList &inputAudioPortNames, const QStringList &outputAudioPortNames, double sampleRate) :
    MidiProcessor(inputAudioPortNames, outputAudioPortNames, sampleRate)
{
}

EventProcessor::EventProcessor(const EventProcessor &tocopy) :
    MidiProcessor(tocopy)
{
}
