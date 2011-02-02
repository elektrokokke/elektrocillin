#include "iirbutterworthfilter.h"
#include <cmath>

IIRButterworthFilter::IIRButterworthFilter(double cutoffFrequencyInHertz, double sampleRate, Type type_, int zeros) :
    IIRFilter(1 + zeros, 2, sampleRate),
//    IIRFilter(1 + zeros, 0, sampleRate),
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
    double factor = 1.0 / (c*c + c*sqrt(2.0) + 1.0);
    setFeedBackCoefficient(0, (-c*c*2.0 + 2.0) * factor);
    setFeedBackCoefficient(1, (c*c - c*sqrt(2.0) + 1.0) * factor);

    int n = getFeedForwardCoefficientCount() - 1;
    int powerOfTwo = 1 << n;
    factor *= 4.0 / powerOfTwo;
//    double factor = 1.0 / powerOfTwo;
    for (int k = 0; k < (n + 2) / 2; k++) {
        setFeedForwardCoefficient(k, factor * IIRFilter::computeBinomialCoefficient(n, k));
    }
    for (int k = (n + 2) / 2; k <= n; k++) {
        setFeedForwardCoefficient(k, getFeedForwardCoefficient(n - k));
    }

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
