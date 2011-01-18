#ifndef MONOPHONICSYNTHESIZER_H
#define MONOPHONICSYNTHESIZER_H

#include "morphingpiecewiselinearoscillator.h"
#include "simplelowpassfilter.h"
#include "sawtoothoscillator.h"
#include "pulseoscillator.h"
#include "adsrenvelope.h"
#include <QStack>

class MonophonicSynthesizer : public AudioSource
{
public:
    MonophonicSynthesizer();

    void pushNote(unsigned char midiNoteNumber);
    void popNote(unsigned char midiNoteNumber);

    double nextSample();

    virtual void setSampleRate(double sampleRate);
    void setFrequency(double frequency);

private:
    PulseOscillator osc;
    PulseOscillator pulseOsc1, pulseOsc2;
    SawtoothOscillator sawOsc1, sawOsc2;
    MorphingPiecewiseLinearOscillator morphOsc1, morphOsc2, morphOsc3;
    Oscillator lfo, lfo2, lfo3;
    AdsrEnvelope envelope;
    QStack<double> frequencies;
    QStack<unsigned char> midiNoteNumbers;

    double computeFrequencyFromMidiNoteNumber(unsigned char midiNoteNumber);
};

#endif // MONOPHONICSYNTHESIZER_H
