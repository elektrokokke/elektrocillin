#ifndef NOTETRIGGERED_H
#define NOTETRIGGERED_H

#include "audioprocessor.h"

class MidiProcessor : public AudioProcessor
{
public:
    MidiProcessor(int nrOfInputs = 0, int nrOfOutputs = 0, double sampleRate = 44100);

    virtual void processNoteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity) = 0;
    virtual void processNoteOff(unsigned char channel, unsigned char noteNumber, unsigned char velocity) = 0;
    virtual void processController(unsigned char channel, unsigned char processController, unsigned char value);
    virtual void processPitchBend(unsigned char channel, unsigned int value);
};

#endif // NOTETRIGGERED_H
