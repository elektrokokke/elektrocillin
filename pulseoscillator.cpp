#include "pulseoscillator.h"
#include <cmath>

PulseOscillator::PulseOscillator(double frequencyModulationIntensity, double sampleRate) :
    LinearOscillator(frequencyModulationIntensity, sampleRate)
{
    // initialize the square wave control points:
    LinearInterpolator pulse(QVector<double>(4), QVector<double>(4));
    pulse.getX()[0] = 0;
    pulse.getY()[0] = -1;
    pulse.getX()[1] = 0.5;
    pulse.getY()[1] = -1;
    pulse.getX()[2] = 0.5;
    pulse.getY()[2] = 1;
    pulse.getX()[3] = 1;
    pulse.getY()[3] = 1;
    setLinearInterpolator(pulse);
}
