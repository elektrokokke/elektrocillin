#ifndef OSCILLATOR_H
#define OSCILLATOR_H

#include "audiosource.h"
#include "simplelowpassfilter.h"

class Oscillator : public AudioSource
{
public:
    Oscillator(bool quantizeFrequency = true);

    virtual void setSampleRate(double sampleRate);

    void setFrequency(double frequency);
    double getFrequency() const;

    double nextSample();

    void reset();

protected:
    virtual double valueAtPhase(double phase);

private:
    bool quantizeFrequency;
    double frequency, phase, phaseIncrement;
    SimpleLowpassFilter filter;

    void computePhaseIncrement();
};

#endif // OSCILLATOR_H
