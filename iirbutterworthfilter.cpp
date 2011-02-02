#include "iirbutterworthfilter.h"
#include <cmath>

IIRButterworthFilter::IIRButterworthFilter(double cutoffFrequencyInHertz, double sampleRate, Type type_) :
    IIRFilter(3, 2, sampleRate),
    type(type_)
{
    setCutoffFrequency(cutoffFrequencyInHertz);
}

void IIRButterworthFilter::setCutoffFrequency(double cutoffFrequencyInHertz, Type type)
{
    this->cutoffFrequency = cutoffFrequencyInHertz;
    this->type = type;
    double radians = convertHertzToRadians(type == LOW_PASS ? cutoffFrequency : getSampleRate() * 0.5 - cutoffFrequencyInHertz);
    double c = cos(radians * 0.5) / sin(radians * 0.5);
    double factor = 1 / (c*c + c*sqrt(2.0) + 1.0);
    getFeedBackCoefficients()[0] = (-c*c*2.0 + 2.0) * factor;
    getFeedBackCoefficients()[1] = (c*c - c*sqrt(2.0) + 1.0) * factor;
    getFeedForwardCoefficients()[0] = factor;
    getFeedForwardCoefficients()[1] = 2 * factor;
    getFeedForwardCoefficients()[2] = factor;
    if (type == HIGH_PASS) {
        invert();
    }
}

void IIRButterworthFilter::setCutoffFrequency(double cutoffFrequencyInHertz)
{
    setCutoffFrequency(cutoffFrequencyInHertz, getType());
}

void IIRButterworthFilter::setType(Type type)
{
    setCutoffFrequency(getCutoffFrequency(), type);
}

double IIRButterworthFilter::getCutoffFrequency() const
{
    return cutoffFrequency;
}

IIRButterworthFilter::Type IIRButterworthFilter::getType() const
{
    return type;
}
