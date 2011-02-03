#ifndef NOTETRIGGERED_H
#define NOTETRIGGERED_H

#include "audioprocessor.h"

class MidiProcessor : public AudioProcessor
{
public:
    MidiProcessor(int nrOfAudioInputs, int nrOfAudioOutputs, double sampleRate = 44100);

    virtual void processNoteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity);
    virtual void processNoteOff(unsigned char channel, unsigned char noteNumber, unsigned char velocity);
    virtual void processController(unsigned char channel, unsigned char processController, unsigned char value);
    virtual void processPitchBend(unsigned char channel, unsigned int value);
};

#endif // NOTETRIGGERED_H
