#include "pulseoscillator.h"
#include <cmath>

PulseOscillator::PulseOscillator(double frequencyModulationIntensity, double sampleRate) :
    LinearOscillator(frequencyModulationIntensity, sampleRate)
{
    // initialize the square wave control points:
    Interpolator::ChangeAllControlPointsEvent event;
    event.xx.resize(4);
    event.yy.resize(4);
    event.xx[0] = 0;
    event.yy[0] = -1;
    event.xx[1] = 0.5;
    event.yy[1] = -1;
    event.xx[2] = 0.5;
    event.yy[2] = 1;
    event.xx[3] = 1;
    event.yy[3] = 1;
    LinearOscillator::processEvent(&event, 0);
}
