#include "pulseoscillator.h"

PulseOscillator::PulseOscillator(double pulseWidth)
{
    Q_ASSERT((pulseWidth >= 0.0) && (pulseWidth <= 2.0 * M_PI));
    addNode(0.0, -1.0);
    addNode(pulseWidth, -1.0);
    addNode(pulseWidth, 1.0);
    addNode(2.0 * M_PI, 1.0);
}
