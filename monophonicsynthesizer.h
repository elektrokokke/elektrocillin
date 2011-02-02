#ifndef MONOPHONICSYNTHESIZER_H
#define MONOPHONICSYNTHESIZER_H

#include "morphingpiecewiselinearoscillator.h"
#include "simplelowpassfilter.h"
#include "sawtoothoscillator.h"
#include "pulseoscillator.h"
#include "adsrenvelope.h"
#include "butterworth2polefilter.h"
#include <QStack>

class MonophonicSynthesizer : public NoteTriggered
{
public:
    MonophonicSynthesizer(double sampleRate = 441000);

    void setSampleRate(double sampleRate);
    void noteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity);
    void noteOff(unsigned char channel, unsigned char noteNumber, unsigned char velocity);
    void pitchBend(unsigned char channel, unsigned int pitch);
    void controller(unsigned char controller, unsigned char value);
    void process(const double *inputs, double *outputs);


private:
    double morph;
    PulseOscillator osc1;
//    SawtoothOscillator osc2;
//    MorphingPiecewiseLinearOscillator morphOsc1;
    AdsrEnvelope envelope;
//    Butterworth2PoleFilter filterAudio, filterMorph;
    QStack<unsigned char> midiNoteNumbers;
};

#endif // MONOPHONICSYNTHESIZER_H
