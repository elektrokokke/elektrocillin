#ifndef NOTETRIGGERED_H
#define NOTETRIGGERED_H

#include "audioprocessor.h"

class MidiProcessor : public AudioProcessor
{
public:
    MidiProcessor(const QStringList &inputAudioPortNames, const QStringList &outputAudioPortNames, double sampleRate = 44100);

    virtual void processNoteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    virtual void processNoteOff(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    virtual void processAfterTouch(unsigned char channel, unsigned char noteNumber, unsigned char pressure, jack_nframes_t time);
    virtual void processController(unsigned char channel, unsigned char controller, unsigned char value, jack_nframes_t time);
    virtual void processPitchBend(unsigned char channel, unsigned int value, jack_nframes_t time);
    virtual void processChannelPressure(unsigned char channel, unsigned char pressure, jack_nframes_t time);

    static double computeFrequencyFromMidiNoteNumber(unsigned char midiNoteNumber);
    static double computePitchBendFactorFromMidiPitch(unsigned int processPitchBend);
};

#endif // NOTETRIGGERED_H
