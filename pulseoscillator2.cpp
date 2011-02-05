#include "pulseoscillator2.h"
#include <cmath>

PulseOscillator2::PulseOscillator2() :
    previousPhase(0),
    previousIntegralValue(0),
    pulseIntegral(QVector<double>(3), QVector<double>(3))
{
    // initialize the pulse integral control points:
    pulseIntegral.getX()[0] = 0;
    pulseIntegral.getY()[0] = 0;
    pulseIntegral.getX()[1] = M_PI;
    pulseIntegral.getY()[1] = M_PI;
    pulseIntegral.getX()[2] = 2 * M_PI;
    pulseIntegral.getY()[2] = 0;
}

double PulseOscillator2::valueAtPhase(double phase)
{
    // compare current phase with previous phase:
    if (phase < previousPhase) {
        // phase has wrapped around, adjust the previous integral level accordingly:
        previousIntegralValue -= pulseIntegral.getY()[2];
        previousPhase -= 2 * M_PI;
    }
    double integralValue = pulseIntegral.evaluate(phase);
    // integral difference / phase difference is the oscillator output:
    double output = (integralValue - previousIntegralValue) / (phase - previousPhase);
    previousIntegralValue = integralValue;
    previousPhase = phase;
    return output;
}
