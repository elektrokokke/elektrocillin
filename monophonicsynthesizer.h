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
    void setController(unsigned char controller, unsigned char value);

    double nextSample();

    virtual void setSampleRate(double sampleRate);

protected:
    void setFrequency(double frequency, double pitchBendFactor);
private:
    double frequency, pitchBendFactor;
    PulseOscillator pulseOsc1, pulseOsc2;
    MorphingPiecewiseLinearOscillator morphOsc1;
    AdsrEnvelope envelope;
    QStack<double> frequencies;
    QStack<unsigned char> midiNoteNumbers;

    double computeFrequencyFromMidiNoteNumber(unsigned char midiNoteNumber);
    double computePitchBendFactorFromMidiPitch(unsigned int pitch);
};

#endif // MONOPHONICSYNTHESIZER_H
