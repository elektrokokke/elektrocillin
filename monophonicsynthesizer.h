#ifndef MONOPHONICSYNTHESIZER_H
#define MONOPHONICSYNTHESIZER_H

#include "morphingpiecewiselinearoscillator.h"
#include "simplelowpassfilter.h"
#include "sawtoothoscillator.h"
#include "pulseoscillator.h"
#include "adsrenvelope.h"
#include "iirbutterworthfilter.h"
#include <QStack>

class MonophonicSynthesizer : public MidiProcessor
{
public:
    MonophonicSynthesizer(double sampleRate = 441000);

    void setSampleRate(double sampleRate);
    void processNoteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    void processNoteOff(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    void processPitchBend(unsigned char channel, unsigned int pitch, jack_nframes_t time);
    void processController(unsigned char processController, unsigned char value, jack_nframes_t time);
    void processAudio(const double *inputs, double *outputs, jack_nframes_t time);


private:
    double morph;
    PulseOscillator osc1;
    SawtoothOscillator osc2;
    MorphingPiecewiseLinearOscillator morphOsc1;
    AdsrEnvelope envelope;
    IirButterworthFilter filterAudio, filterMorph;
    QStack<unsigned char> midiNoteNumbers;
};

#endif // MONOPHONICSYNTHESIZER_H
