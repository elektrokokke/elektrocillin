#ifndef MONOPHONICSYNTHESIZER_H
#define MONOPHONICSYNTHESIZER_H

#include "pulseoscillator.h"
#include "simplelowpassfilter.h"
#include "adsrenvelope.h"
#include <QStack>

class MonophonicSynthesizer : public AudioSource
{
public:
    MonophonicSynthesizer();

    void pushNote(unsigned char midiNoteNumber);
    void popNote(unsigned char midiNoteNumber);

    double nextSample();

private:
    PulseOscillator oscillator;
    AdsrEnvelope envelope;
    SimpleLowpassFilter filter;
    QStack<double> frequencies;
    QStack<unsigned char> midiNoteNumbers;

    double computeFrequencyFromMidiNoteNumber(unsigned char midiNoteNumber);
};

#endif // MONOPHONICSYNTHESIZER_H
