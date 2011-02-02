#ifndef NOTETRIGGERED_H
#define NOTETRIGGERED_H

#include "sampled.h"

class NoteTriggered : public Sampled
{
public:
    NoteTriggered(int nrOfInputs = 0, int nrOfOutputs = 0, double sampleRate = 44100);

    virtual void noteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity) = 0;
    virtual void noteOff(unsigned char channel, unsigned char noteNumber, unsigned char velocity) = 0;
    virtual void controller(unsigned char channel, unsigned char controller, unsigned char value);
    virtual void pitchBend(unsigned char channel, unsigned int value);
};

#endif // NOTETRIGGERED_H
