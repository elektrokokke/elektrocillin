#ifndef OSCILLATOR_H
#define OSCILLATOR_H

#include "midiprocessor.h"

class Oscillator : public MidiProcessor
{
public:
    Oscillator(double sampleRate = 44100);

    virtual void setSampleRate(double sampleRate);
    virtual void processNoteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    virtual void processPitchBend(unsigned char channel, unsigned int value, jack_nframes_t time);
    virtual void processAudio(const double *inputs, double *outputs, jack_nframes_t time);

    void setFrequency(double hertz);
    double getFrequency() const;
    double getPhaseIncrement() const;

protected:
    virtual double valueAtPhase(double phase);

private:
    unsigned char noteNumber;
    int pitchBendValue;
    double frequencyInHertz, phase, phaseIncrement;

    void computePhaseIncrement();
};

#endif // OSCILLATOR_H
