#include "iirbutterworthfilter.h"
#include <cmath>

IIRButterworthFilter::IIRButterworthFilter(double cutoffFrequencyInHertz, double sampleRate) :
    IIRFilter(3, 2, sampleRate)
{
    setCutoffFrequency(cutoffFrequencyInHertz);
}

void IIRButterworthFilter::setCutoffFrequency(double cutoffFrequencyInHertz)
{
    double radians = getFrequencyInRadians(cutoffFrequencyInHertz);
    double c = cos(radians * 0.5) / sin(radians * 0.5);
    double factor = 1.0 / (c*c + c*sqrt(2.0) + 1.0);
    setFeedForwardCoefficient(0, 1.0 * factor);
    setFeedForwardCoefficient(1, 2.0 * factor);
    setFeedForwardCoefficient(2, 1.0 * factor);
    setFeedBackCoefficient(0, (-c*c*2.0 + 2.0) * factor);
    setFeedBackCoefficient(1, (c*c - c*sqrt(2.0) + 1.0) * factor);
}
