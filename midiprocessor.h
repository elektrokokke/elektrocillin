#ifndef NOTETRIGGERED_H
#define NOTETRIGGERED_H

#include "audioprocessor.h"

class MidiProcessor : public AudioProcessor
{
public:
    MidiProcessor(int nrOfAudioInputs, int nrOfAudioOutputs, double sampleRate = 44100);

    virtual void processNoteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    virtual void processNoteOff(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    virtual void processController(unsigned char channel, unsigned char processController, unsigned char value, jack_nframes_t time);
    virtual void processPitchBend(unsigned char channel, unsigned int value, jack_nframes_t time);

    static double computeFrequencyFromMidiNoteNumber(unsigned char midiNoteNumber);
    static double computePitchBendFactorFromMidiPitch(unsigned int processPitchBend);
};

#endif // NOTETRIGGERED_H
