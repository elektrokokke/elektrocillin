#include "zplanefilter.h"
#include "polynomial.h"
#include <cmath>
#include <QDebug>

ZPlaneFilter::ZPlaneFilter() :
    gainFactor(1.0)
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
        y[0] -= y[i] * feedbackCoefficients[i];
    }
    y[0] *= gainFactor;
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

size_t ZPlaneFilter::poleCount() const
{
    return poles.size();
}

size_t ZPlaneFilter::zeroCount() const
{
    return zeros.size();
}

std::complex<double> & ZPlaneFilter::pole(size_t i)
{
    return poles[i];
}

std::complex<double> & ZPlaneFilter::zero(size_t i)
{
    return zeros[i];
}

void ZPlaneFilter::setGainFactor(double gainFactor)
{
    this->gainFactor = gainFactor;
}

double ZPlaneFilter::getGainFactor() const
{
    return gainFactor;
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
    return gainFactor * numerator / denominator;
}

void ZPlaneFilter::computeCoefficients()
{
    std::complex<double> one(1.0, 0.0);
    // compute numerator and denominator:
    Polynomial<std::complex<double> > numerator(one);
    for (size_t i = 0; i < zeros.size(); i++) {
        Polynomial<std::complex<double> > zero(one, -zeros[i]);
        qDebug() << "(" << zero[0].real() << "," << zero[0].imag() << "* i) + (" << zero[1].real() << "," << zero[1].imag() << "* i) * z^-1";
        numerator *= zero;
    }
    Polynomial<std::complex<double> > denominator(one);
    for (size_t i = 0; i < poles.size(); i++) {
        Polynomial<std::complex<double> > pole(one, -poles[i]);
        qDebug() << "(" << pole[0].real() << "," << pole[0].imag() << "* i) + (" << pole[1].real() << "," << pole[1].imag() << "* i) * z^-1";
        denominator *= pole;
    }
    // note: feedforwardCoefficients[0] and feedbackCoefficients[0] will always be 1
    feedforwardCoefficients = numerator;
    feedbackCoefficients = denominator;
    qDebug() << "numerator";
    for (size_t i = 0; i < feedforwardCoefficients.size(); i++) {
        qDebug() << feedforwardCoefficients[i].real();
    }
    qDebug() << "denominator";
    for (size_t i = 0; i < feedbackCoefficients.size(); i++) {
        qDebug() << feedbackCoefficients[i].real();
    }
}

double ZPlaneFilter::convertPowerToDecibel(double power)
{
    // follow the 10 * log10 rule:
    return 10.0 * log(power) / log(10.0);
}
