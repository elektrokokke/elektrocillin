#ifndef PULSEOSCILLATOR_H
#define PULSEOSCILLATOR_H

#include "piecewiselinearoscillator.h"
#include <cmath>

class PulseOscillator : public PiecewiseLinearOscillator
{
public:
    PulseOscillator(double pulseWidth = M_PI);
};

#endif // PULSEOSCILLATOR_H
