#include "sawtoothoscillator.h"
#include <cmath>

SawtoothOscillator::SawtoothOscillator(double peak)
{
    Q_ASSERT((peak >= 0.0) && (peak <= M_PI));
    addNode(QPointF(peak, -1.0));
    addNode(QPointF(peak, -1.0));
    addNode(QPointF(2.0 * M_PI - peak, 1.0));
    addNode(QPointF(2.0 * M_PI - peak, 1.0));
}
