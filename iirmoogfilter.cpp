#include "iirmoogfilter.h"
#include <cmath>

IIRMoogFilter::IIRMoogFilter(double cutoffFrequencyInHertz, double resonance, double sampleRate, int zeros) :
    IIRFilter(1 + zeros, 4, sampleRate)
{
    setCutoffFrequency(cutoffFrequencyInHertz, resonance);
}

void IIRMoogFilter::setCutoffFrequency(double cutoffFrequencyInHertz, double resonance)
{
    this->cutoffFrequencyInHertz = cutoffFrequencyInHertz;
    this->resonance = resonance;
    double radians = convertHertzToRadians(cutoffFrequencyInHertz);
    double s = sin(radians);
    double c = cos(radians);
    double t = tan((radians - M_PI) * 0.25);
    double a1 = t / (s - c * t);
    double g1Square_inv = 1.0 + a1 * a1 + 2.0 * a1 * c;
    double k = resonance * g1Square_inv * g1Square_inv;
    setFeedBackCoefficient(0, k + 4.0 * a1);
    setFeedBackCoefficient(1, 6.0 * a1 * a1);
    setFeedBackCoefficient(2, 4.0 * a1 * a1 * a1);
    setFeedBackCoefficient(3, a1 * a1 * a1 * a1);

    int n = getFeedForwardCoefficientCount() - 1;
    double feedBackSum = 1.0 + getFeedBackCoefficient(0) + getFeedBackCoefficient(1) + getFeedBackCoefficient(2) + getFeedBackCoefficient(3);
    int powerOfTwo = 1 << n;
    double factor = 1.0 / powerOfTwo;
    for (int k = 0; k < (n + 2) / 2; k++) {
        setFeedForwardCoefficient(k, factor * IIRFilter::computeBinomialCoefficient(n, k) * feedBackSum);
    }
    for (int k = (n + 2) / 2; k <= n; k++) {
        setFeedForwardCoefficient(k, getFeedForwardCoefficient(n - k));
    }
}

void IIRMoogFilter::setSampleRate(double sampleRate)
{
    IIRFilter::setSampleRate(sampleRate);
    // recompute coefficients:
    setCutoffFrequency(getCutoffFrequency(), getResonance());
}

void IIRMoogFilter::setCutoffFrequency(double cutoffFrequencyInHertz)
{
    setCutoffFrequency(cutoffFrequencyInHertz, getResonance());
}

void IIRMoogFilter::setResonance(double resonance)
{
    setCutoffFrequency(getCutoffFrequency(), resonance);
}

double IIRMoogFilter::getCutoffFrequency() const
{
    return cutoffFrequencyInHertz;
}
double IIRMoogFilter::getResonance() const
{
    return resonance;
}
