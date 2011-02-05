#ifndef PULSEOSCILLATOR2_H
#define PULSEOSCILLATOR2_H

#include "linearinterpolator.h"
#include "oscillator.h"

class PulseOscillator2 : public Oscillator
{
public:
    PulseOscillator2();

protected:
    double valueAtPhase(double phase);

private:
    double previousPhase, previousIntegralValue;
    LinearInterpolator pulseIntegral;
};

#endif // PULSEOSCILLATOR2_H
