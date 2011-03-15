#include "midiprocessor.h"
#include <cmath>

MidiProcessor::~MidiProcessor()
{
}

double MidiProcessor::computeFrequencyFromMidiNoteNumber(unsigned char midiNoteNumber)
{
    // 440 Hz is note number 69:
    double octave = ((double)midiNoteNumber - 69.0) / 12.0;
    double frequency = 440.0 * pow(2.0, octave);
    return frequency;
}

double MidiProcessor::computePitchBendFactorFromMidiPitch(double base, unsigned int pitchBend)
{
    // center it around 0x2000:
    int pitchCentered = (int)pitchBend - 0x2000;
    // -8192 means minus two half tones => -49152 is one octave => factor 2^(-1)
    // +8192 means plus two half tones => +49152 is one octave => factor 2^1
    return pow(base, (double)pitchCentered / 49152.0);
}

void MidiProcessor::processNoteOff(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time)
{
}

void MidiProcessor::processAfterTouch(unsigned char channel, unsigned char noteNumber, unsigned char pressure, jack_nframes_t time)
{
}

void MidiProcessor::processController(unsigned char channel, unsigned char controller, unsigned char value, jack_nframes_t time)
{
}

void MidiProcessor::processPitchBend(unsigned char channel, unsigned int value, jack_nframes_t time)
{
}

void MidiProcessor::processChannelPressure(unsigned char channel, unsigned char pressure, jack_nframes_t time)
{
}
