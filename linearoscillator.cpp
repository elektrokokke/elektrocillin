#include "linearoscillator.h"
#include <cmath>

LinearOscillator::LinearOscillator(const LinearInterpolator *interpolator, double frequencyModulationIntensity, double sampleRate) :
        Oscillator(frequencyModulationIntensity, sampleRate),
        integral(interpolator)
{
}

void LinearOscillator::setLinearInterpolator(const LinearInterpolator *interpolator)
{
    integral = LinearIntegralInterpolator(interpolator);
}

double LinearOscillator::valueAtPhase(double phase)
{
    if (phase == previousPhase) {
        return 0;
    }
    // compare current phase with previous phase:
    if (phase < previousPhase) {
        // phase has wrapped around, adjust the previous integral level accordingly:
        previousIntegralValue -= integral.interpolate(integral.getX().size() - 2, integral.getX().back());
        previousPhase -= 2 * M_PI;
    }
    double integralValue = integral.evaluate(phase);
    // integral difference / phase difference is the oscillator output:
    double output = (integralValue - previousIntegralValue) / (phase - previousPhase);
    previousIntegralValue = integralValue;
    previousPhase = phase;
    return output;
}
