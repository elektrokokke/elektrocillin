#include "midiprocessor.h"

MidiProcessor::MidiProcessor(int nrOfInputs, int nrOfOutputs, double sampleRate) :
    AudioProcessor(nrOfInputs, nrOfOutputs, sampleRate)
{
}

void MidiProcessor::processController(unsigned char, unsigned char, unsigned char)
{
}

void MidiProcessor::processPitchBend(unsigned char, unsigned int)
{
}
