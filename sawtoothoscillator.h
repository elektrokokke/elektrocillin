#ifndef SAWTOOTHOSCILLATOR_H
#define SAWTOOTHOSCILLATOR_H

#include "piecewiselinearoscillator.h"

class SawtoothOscillator : public PiecewiseLinearOscillator
{
public:
    SawtoothOscillator(double peak = 0.0, double frequencyModulationIntensity = 2.0/12.0, double sampleRate = 44100);
};

#endif // SAWTOOTHOSCILLATOR_H
