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

    void setMidiPitch(unsigned int pitch);

    double nextSample();

    virtual void setSampleRate(double sampleRate);

protected:
    void setFrequency(double frequency, double pitchBendFactor);
private:
    double frequency, pitchBendFactor;
    PulseOscillator osc;
    PulseOscillator pulseOsc1, pulseOsc2;
    SawtoothOscillator sawOsc1, sawOsc2;
    MorphingPiecewiseLinearOscillator morphOsc1, morphOsc2, morphOsc3;
    Oscillator lfo, lfo2, lfo3;
    AdsrEnvelope envelope;
    QStack<double> frequencies;
    QStack<unsigned char> midiNoteNumbers;

    double computeFrequencyFromMidiNoteNumber(unsigned char midiNoteNumber);
    double computePitchBendFactorFromMidiPitch(unsigned int pitch);
};

#endif // MONOPHONICSYNTHESIZER_H
