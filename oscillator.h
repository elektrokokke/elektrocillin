#ifndef OSCILLATOR_H
#define OSCILLATOR_H

#include "audiosource.h"

class Oscillator : public AudioSource
{
public:
    Oscillator();

    virtual void setSampleRate(double sampleRate);

    void setFrequency(double frequency);
    double getFrequency() const;
    double getPhaseIncrement() const;

    double nextSample();

    void reset();

protected:
    virtual double valueAtPhase(double phase);


private:
    double frequency, phase, phaseIncrement;
    int sampleNr;

    void computePhaseIncrement();
};

#endif // OSCILLATOR_H
