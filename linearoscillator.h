#ifndef LINEAROSCILLATOR_H
#define LINEAROSCILLATOR_H

#include "oscillator.h"
#include "linearintegralinterpolator.h"

class LinearOscillator : public Oscillator
{
public:
    LinearOscillator(const LinearInterpolator *interpolator, double frequencyModulationIntensity = 2.0/12.0, double sampleRate = 44100);

    void setLinearInterpolator(const LinearInterpolator *interpolator);

protected:
    double valueAtPhase(double phase);

private:
    double previousPhase, previousIntegralValue;
    LinearIntegralInterpolator integral;

};

#endif // LINEAROSCILLATOR_H
