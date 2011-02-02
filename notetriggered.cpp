#include "notetriggered.h"

NoteTriggered::NoteTriggered(int nrOfInputs, int nrOfOutputs, double sampleRate) :
    Sampled(nrOfInputs, nrOfOutputs, sampleRate)
{
}

void NoteTriggered::controller(unsigned char channel, unsigned char controller, unsigned char value)
{
}

void NoteTriggered::pitchBend(unsigned char channel, unsigned int value)
{
}
