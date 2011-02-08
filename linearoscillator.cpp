#include "linearoscillator.h"
#include <cmath>

LinearOscillator::LinearOscillator(double frequencyModulationIntensity, double sampleRate) :
        Oscillator(frequencyModulationIntensity, sampleRate),
        previousPhase(0),
        previousIntegralValue(0),
        interpolator(QVector<double>(1), QVector<double>(1)),
        integral(interpolator)
{
    interpolator.getX()[0] = 0;
    interpolator.getY()[0] = -1;
    interpolator.getX().append(M_PI);
    interpolator.getY().append(0);
    interpolator.getX().append(2 * M_PI);
    interpolator.getY().append(1);
    integral = LinearIntegralInterpolator(interpolator);
}

LinearOscillator::LinearOscillator(const LinearInterpolator &interpolator_, double frequencyModulationIntensity, double sampleRate) :
        Oscillator(frequencyModulationIntensity, sampleRate),
        previousPhase(0),
        previousIntegralValue(0),
        interpolator(interpolator_),
        integral(interpolator_)
{
}

const LinearInterpolator & LinearOscillator::getLinearInterpolator() const
{
    return interpolator;
}

void LinearOscillator::setLinearInterpolator(const LinearInterpolator &interpolator)
{
    this->interpolator = interpolator;
    integral = LinearIntegralInterpolator(interpolator);
}

const LinearIntegralInterpolator & LinearOscillator::getLinearIntegralInterpolator() const
{
    return integral;
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
