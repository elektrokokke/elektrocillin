#ifndef PULSEOSCILLATOR2_H
#define PULSEOSCILLATOR2_H

#include "linearoscillator.h"

class PulseOscillator : public LinearOscillator
{
public:
    PulseOscillator(double frequencyModulationIntensity = 2.0/12.0, double sampleRate = 44100);
};

#endif // PULSEOSCILLATOR2_H
