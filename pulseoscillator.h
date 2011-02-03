#ifndef PULSEOSCILLATOR_H
#define PULSEOSCILLATOR_H

#include "piecewiselinearoscillator.h"
#include <cmath>

class PulseOscillator : public PiecewiseLinearOscillator
{
public:
    PulseOscillator(double pulseWidth = M_PI, double frequencyModulationIntensity = 2.0/12.0, double sampleRate = 44100);
};

#endif // PULSEOSCILLATOR_H
