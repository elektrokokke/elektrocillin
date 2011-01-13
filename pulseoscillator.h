#ifndef PULSEOSCILLATOR_H
#define PULSEOSCILLATOR_H

#include "oscillator.h"

class PulseOscillator : public Oscillator
{
public:
    PulseOscillator();

protected:
    virtual double valueAtPhase(double phase);
};

#endif // PULSEOSCILLATOR_H
