#ifndef SAWTOOTHOSCILLATOR_H
#define SAWTOOTHOSCILLATOR_H

#include "piecewiselinearoscillator.h"

class SawtoothOscillator : public PiecewiseLinearOscillator
{
public:
    SawtoothOscillator(double peak = 0.0);
};

#endif // SAWTOOTHOSCILLATOR_H
