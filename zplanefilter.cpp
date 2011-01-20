#include "zplanefilter.h"
#include "polynomial.h"
#include <cmath>

ZPlaneFilter::ZPlaneFilter()
{
}

double ZPlaneFilter::filter(double x0)
{
    // move previous x and y values one step back:
    x.insert(x.begin(), 1, std::complex<double>(x0, 0.0));
    y.insert(y.begin(), 1, std::complex<double>(0.0, 0.0));
    // we just need as many previous values/results as we have coefficients:
    if (x.size() != feedforwardCoefficients.size()) {
        x.resize(feedforwardCoefficients.size());
    }
    if (y.size() != feedbackCoefficients.size()) {
        y.resize(feedbackCoefficients.size());
    }
    // compute the current value:
    for (size_t i = 0; i < feedforwardCoefficients.size(); i++) {
        y[0] += x[i] * feedforwardCoefficients[i];
    }
    for (size_t i = 1; i < feedbackCoefficients.size(); i++) {
        y[0] += y[i] * feedbackCoefficients[i];
    }
    return y[0].real();
}

void ZPlaneFilter::addPole(const std::complex<double> &pole)
{
    poles.push_back(pole);
}

void ZPlaneFilter::addZero(const std::complex<double> &zero)
{
    zeros.push_back(zero);
}

std::complex<double> & ZPlaneFilter::pole(size_t i)
{
    return poles[i];
}

std::complex<double> & ZPlaneFilter::zero(size_t i)
{
    return zeros[i];
}

double ZPlaneFilter::squaredAmplitudeResponse(double frequencyInRadians)
{
    // compute the squared amplitude response (power) from the frequency response:
    return std::norm(frequencyResponse(frequencyInRadians));
}

std::complex<double> ZPlaneFilter::frequencyResponse(double frequencyInRadians)
{
    // convert the frequency to a complex number on the unit circle:
    return frequencyResponse(std::polar(1.0, frequencyInRadians));
}

std::complex<double> ZPlaneFilter::frequencyResponse(const std::complex<double> z)
{
    // evaluate based on the pole/zero representation:
    std::complex<double> numerator(1.0, 0.0), denominator(1.0, 0.0);
    for (size_t i = 0; i < zeros.size(); i++) {
        numerator *= (z - zeros[i]);
    }
    for (size_t i = 0; i < poles.size(); i++) {
        denominator *= (z - poles[i]);
    }
    return numerator / denominator;
}

void ZPlaneFilter::computeCoefficients()
{
    std::complex<double> one(1.0, 0.0);
    // compute numerator and denominator:
    Polynomial<std::complex<double> > numerator(one);
    for (size_t i = 0; i < zeros.size(); i++) {
        numerator *= Polynomial<std::complex<double> >(one, -zeros[i]);
    }
    Polynomial<std::complex<double> > denominator(one);
    for (size_t i = 0; i < poles.size(); i++) {
        denominator *= Polynomial<std::complex<double> >(one, -poles[i]);
    }
    // normalize by the constant in the denominator polynomial:
    numerator *= one / denominator[0];
    denominator *= one / denominator[0];
    feedforwardCoefficients = numerator;
    feedbackCoefficients = denominator;
}

double ZPlaneFilter::convertPowerToDecibel(double power)
{
    // follow the 10 * log10 rule:
    return 10.0 * log(power) / log(10.0);
}
