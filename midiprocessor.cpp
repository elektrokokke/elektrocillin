#include "midiprocessor.h"
#include <cmath>

MidiProcessor::MidiProcessor(const QStringList &inputPortNames, const QStringList &outputPortNames, double sampleRate) :
    AudioProcessor(inputPortNames, outputPortNames, sampleRate)
{
}

void MidiProcessor::processNoteOn(unsigned char, unsigned char, unsigned char, jack_nframes_t)
{
}

void MidiProcessor::processNoteOff(unsigned char, unsigned char, unsigned char, jack_nframes_t)
{
}

void MidiProcessor::processAfterTouch(unsigned char, unsigned char, unsigned char, jack_nframes_t)
{
}

void MidiProcessor::processController(unsigned char, unsigned char, unsigned char, jack_nframes_t)
{
}

void MidiProcessor::processPitchBend(unsigned char, unsigned int, jack_nframes_t)
{
}

void MidiProcessor::processChannelPressure(unsigned char, unsigned char, jack_nframes_t)
{
}

double MidiProcessor::computeFrequencyFromMidiNoteNumber(unsigned char midiNoteNumber)
{
    // 440 Hz is note number 69:
    double octave = ((double)midiNoteNumber - 69.0) / 12.0;
    double frequency = 440.0 * pow(2.0, octave);
    return frequency;
}

double MidiProcessor::computePitchBendFactorFromMidiPitch(unsigned int pitchBend)
{
    // center it around 0x2000:
    int pitchCentered = (int)pitchBend - 0x2000;
    // -8192 means minus two half tones => -49152 is one octave => factor 2^(-1)
    // +8192 means plus two half tones => +49152 is one octave => factor 2^1
    return pow(2.0, (double)pitchCentered / 49152.0);
}
