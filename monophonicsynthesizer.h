#ifndef MONOPHONICSYNTHESIZER_H
#define MONOPHONICSYNTHESIZER_H

#include "pulseoscillator.h"
#include "simplelowpassfilter.h"
#include <QStack>

class MonophonicSynthesizer
{
public:
    MonophonicSynthesizer();

    void setSampleRate(double sampleRate);
    double getSampleRate() const;

    void pushNote(unsigned char midiNoteNumber);
    void popNote(unsigned char midiNoteNumber);

    double nextSample();

private:
    int attack, release;
    PulseOscillator oscillator;
    QStack<double> frequencies;
    QStack<unsigned char> midiNoteNumbers;

    double computeFrequencyFromMidiNoteNumber(unsigned char midiNoteNumber);
};

#endif // MONOPHONICSYNTHESIZER_H
