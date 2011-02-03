#include "pulseoscillator.h"

PulseOscillator::PulseOscillator(double pulseWidth, double frequencyModulationIntensity, double sampleRate) :
    PiecewiseLinearOscillator(frequencyModulationIntensity, sampleRate)
{
    Q_ASSERT((pulseWidth >= 0.0) && (pulseWidth <= 2.0 * M_PI));
    addNode(QPointF(0.0, -1.0));
    addNode(QPointF(pulseWidth, -1.0));
    addNode(QPointF(pulseWidth, 1.0));
    addNode(QPointF(2.0 * M_PI, 1.0));
}
