#ifndef MONOPHONICSYNTHESIZER_H
#define MONOPHONICSYNTHESIZER_H

#include "morphingpiecewiselinearoscillator.h"
#include "simplelowpassfilter.h"
#include "sawtoothoscillator.h"
#include "pulseoscillator.h"
#include "adsrenvelope.h"
#include "butterworth2polefilter.h"
#include <QStack>

class MonophonicSynthesizer : public MidiProcessor
{
public:
    MonophonicSynthesizer(double sampleRate = 441000);

    void setSampleRate(double sampleRate);
    void processNoteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity);
    void processNoteOff(unsigned char channel, unsigned char noteNumber, unsigned char velocity);
    void processPitchBend(unsigned char channel, unsigned int pitch);
    void processController(unsigned char processController, unsigned char value);
    void processAudio(const double *inputs, double *outputs);


private:
    double morph;
    SawtoothOscillator osc1;
//    SawtoothOscillator osc2;
//    MorphingPiecewiseLinearOscillator morphOsc1;
    AdsrEnvelope envelope;
//    Butterworth2PoleFilter filterAudio, filterMorph;
    QStack<unsigned char> midiNoteNumbers;
};

#endif // MONOPHONICSYNTHESIZER_H
