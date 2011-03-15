#ifndef NOTETRIGGERED_H
#define NOTETRIGGERED_H

#include <jack/types.h>

class MidiProcessor
{
public:
    virtual ~MidiProcessor();

    virtual void processNoteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time) = 0;
    virtual void processNoteOff(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    virtual void processAfterTouch(unsigned char channel, unsigned char noteNumber, unsigned char pressure, jack_nframes_t time);
    virtual void processController(unsigned char channel, unsigned char controller, unsigned char value, jack_nframes_t time);
    virtual void processPitchBend(unsigned char channel, unsigned int value, jack_nframes_t time);
    virtual void processChannelPressure(unsigned char channel, unsigned char pressure, jack_nframes_t time);

    static double computeFrequencyFromMidiNoteNumber(unsigned char midiNoteNumber);
    static double computePitchBendFactorFromMidiPitch(double base, unsigned int processPitchBend);
};

#endif // NOTETRIGGERED_H
