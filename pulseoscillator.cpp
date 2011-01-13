#include "pulseoscillator.h"
#include <cmath>

PulseOscillator::PulseOscillator()
{
}

double PulseOscillator::valueAtPhase(double phase)
{
    return (phase < M_PI ? 1.0 : -1.0);
}
