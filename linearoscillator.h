#ifndef LINEAROSCILLATOR_H
#define LINEAROSCILLATOR_H

#include "oscillator.h"
#include "linearintegralinterpolator.h"

class LinearOscillator : public Oscillator
{
public:
    LinearOscillator(double frequencyModulationIntensity = 2.0/12.0, double sampleRate = 44100);
    /**
      @param interpolator has to span the interval [0..2 * pi]
      */
    LinearOscillator(const LinearInterpolator &interpolator, double frequencyModulationIntensity = 2.0/12.0, double sampleRate = 44100);

    const LinearInterpolator & getLinearInterpolator() const;
    void setLinearInterpolator(const LinearInterpolator &interpolator);
    const LinearIntegralInterpolator & getLinearIntegralInterpolator() const;

protected:
    double valueAtPhase(double phase);

private:
    double previousPhase, previousIntegralValue;
    LinearInterpolator interpolator;
    LinearIntegralInterpolator integral;

};

#endif // LINEAROSCILLATOR_H
