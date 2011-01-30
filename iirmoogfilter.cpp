#include "iirmoogfilter.h"
#include <cmath>

IIRMoogFilter::IIRMoogFilter(double cutoffFrequencyInHertz, double resonance, double sampleRate) :
    IIRFilter(1, 4, sampleRate)
{
    setCutoffFrequency(cutoffFrequencyInHertz, resonance);
}

void IIRMoogFilter::setCutoffFrequency(double cutoffFrequencyInHertz)
{
    setCutoffFrequency(cutoffFrequencyInHertz, resonance);
}

void IIRMoogFilter::setCutoffFrequency(double cutoffFrequencyInHertz, double resonance)
{
    this->cutoffFrequencyInHertz = cutoffFrequencyInHertz;
    this->resonance = resonance;
    double radians = getFrequencyInRadians(cutoffFrequencyInHertz);
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
    setFeedForwardCoefficient(0, 1.0 + getFeedBackCoefficient(0) + getFeedBackCoefficient(1) + getFeedBackCoefficient(2) + getFeedBackCoefficient(3));
}

void IIRMoogFilter::setResonance(double resonance)
{
    setCutoffFrequency(cutoffFrequencyInHertz, resonance);
}
