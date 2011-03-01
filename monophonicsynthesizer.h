#ifndef MONOPHONICSYNTHESIZER_H
#define MONOPHONICSYNTHESIZER_H

#include "simplelowpassfilter.h"
#include "pulseoscillator.h"
#include "envelope.h"
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
    double controller;
    PulseOscillator osc;
    //Oscillator osc;
    Envelope envelope;
    IirButterworthFilter filterAudio, filterController;
    QStack<unsigned char> midiNoteNumbers;
};

#endif // MONOPHONICSYNTHESIZER_H
