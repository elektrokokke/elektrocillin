#include "iirmoogfilter.h"
#include <cmath>

IIRMoogFilter::IIRMoogFilter(double cutoffFrequencyInHertz, double resonance, int nrOfInputs, double sampleRate, int zeros) :
    IIRFilter(1 + zeros, 4, nrOfInputs, sampleRate)
{
    setCutoffFrequency(cutoffFrequencyInHertz, resonance);
}

void IIRMoogFilter::setSampleRate(double sampleRate)
{
    IIRFilter::setSampleRate(sampleRate);
    // recompute coefficients:
    setCutoffFrequency(getCutoffFrequency(), getResonance());
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
    double a2 = a1 * a1;
    double g1Square_inv = 1.0 + a2 + 2.0 * a1 * c;
    double k = resonance * g1Square_inv * g1Square_inv;
    getFeedBackCoefficients()[0] = k + 4.0 * a1;
    getFeedBackCoefficients()[1] = 6.0 * a2;
    getFeedBackCoefficients()[2] = 4.0 * a2 * a1;
    getFeedBackCoefficients()[3] = a2 * a2;

    int n = getFeedForwardCoefficients().size() - 1;
    double feedBackSum = 1.0 + getFeedBackCoefficients()[0] + getFeedBackCoefficients()[1] + getFeedBackCoefficients()[2] + getFeedBackCoefficients()[3];
    int powerOfTwo = 1 << n;
    double factor = 1.0 / powerOfTwo;
    for (int k = 0; k < (n + 2) / 2; k++) {
        getFeedForwardCoefficients()[k] = factor * IIRFilter::computeBinomialCoefficient(n, k) * feedBackSum;
    }
    for (int k = (n + 2) / 2; k <= n; k++) {
        getFeedForwardCoefficients()[k] = getFeedForwardCoefficients()[n - k];
    }
}

double IIRMoogFilter::getCutoffFrequency() const
{
    return cutoffFrequencyInHertz;
}
double IIRMoogFilter::getResonance() const
{
    return resonance;
}
