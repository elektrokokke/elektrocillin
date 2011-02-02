#include "iirfilter.h"
#include <QDebug>

IIRFilter::IIRFilter(double sampleRate_) :
    sampleRate(sampleRate_)
{
}

IIRFilter::IIRFilter(int feedForwardCoefficients, int feedBackCoefficients, double sampleRate_) :
    sampleRate(sampleRate_),
    feedForward(feedForwardCoefficients),
    feedBack(feedBackCoefficients),
    x(feedForwardCoefficients),
    y(feedBackCoefficients)
{
    reset();
}

QString IIRFilter::toString() const
{
    return getNumeratorPolynomial().toString() + " / " + getDenominatorPolynomial().toString();
}

void IIRFilter::setSampleRate(double sampleRate)
{
    this->sampleRate = sampleRate;
}

double IIRFilter::getSampleRate() const
{
    return sampleRate;
}

double IIRFilter::getFrequencyInRadians(double frequencyInHertz) const
{
    return frequencyInHertz * 2.0 * M_PI / getSampleRate();
}

double IIRFilter::filter(double x0)
{
    if (x.size()) {
        x[0] = x0;
        double result = 0.0;
        for (int i = 0; i < x.size(); i++) {
            result += x[i] * feedForward[i];
        }
        for (int i = 0; i < y.size(); i++) {
            result -= y[i] * feedBack[i];
        }
        // remember x and y values for next calls:
        for (int i = x.size() - 1; i > 0; i--) {
            x[i] = x[i - 1];
        }
        for (int i = y.size() - 1; i > 0; i--) {
            y[i] = y[i - 1];
        }
        y[0] = result;
        return result;
    } else {
        return 0;
    }
}

void IIRFilter::reset()
{
    x.fill(0.0);
    y.fill(0.0);
}

double IIRFilter::getSquaredAmplitudeResponse(double hertz)
{
    std::complex<double> z_inv = 1.0 / std::exp(std::complex<double>(0.0, getFrequencyInRadians(hertz)));
    Polynomial<std::complex<double> > numerator = getNumeratorPolynomial();
    Polynomial<std::complex<double> > denominator = getDenominatorPolynomial();
    return std::norm(numerator.evaluate(z_inv) / denominator.evaluate(z_inv));
}

void IIRFilter::addFeedForwardCoefficient(double c)
{
    feedForward.append(c);
    x.append(0);
}

void IIRFilter::addFeedBackCoefficient(double c)
{
    feedBack.append(c);
    y.append(0);
}

int IIRFilter::getFeedForwardCoefficientCount() const
{
    return feedForward.size();
}

int IIRFilter::getFeedBackCoefficientCount() const
{
    return feedBack.size();
}

void IIRFilter::setFeedForwardCoefficient(int index, double c)
{
    feedForward[index] = c;
}

void IIRFilter::setFeedBackCoefficient(int index, double c)
{
    feedBack[index] = c;
}

double IIRFilter::getFeedForwardCoefficient(int index) const
{
    return feedForward[index];
}

double IIRFilter::getFeedBackCoefficient(int index) const
{
    return feedBack[index];
}

void IIRFilter::invert()
{
    // negate all coefficients with odd exponent:
    for (int i = 1; i < feedForward.size(); i += 2) {
        feedForward[i] = -feedForward[i];
    }
    for (int i = 0; i < feedBack.size(); i += 2) {
        feedBack[i] = -feedBack[i];
    }
}

int IIRFilter::computeBinomialCoefficient(int n, int k)
{
    if (k == 0) {
        return 1;
    } else if (2 * k > n) {
        return computeBinomialCoefficient(n, n - k);
    } else {
        int result = n;
        for (int i = 2; i <= k; i++) {
            result *= n + 1 - i;
            result /= i;
        }
        return result;
    }
}

IIRFilter& IIRFilter::operator+=(const IIRFilter &b)
{
    // get numerators and denominators:
    Polynomial<std::complex<double> > numerator1 = getNumeratorPolynomial();
    Polynomial<std::complex<double> > numerator2 = b.getNumeratorPolynomial();
    Polynomial<std::complex<double> > denominator1 = getDenominatorPolynomial();
    Polynomial<std::complex<double> > denominator2 = b.getDenominatorPolynomial();
    // multiply the denominators:
    Polynomial<std::complex<double> > denominator = denominator1;
    denominator *= denominator2;
    // multiply the numerators with the other filter's denominator:
    numerator1 *= denominator2;
    numerator2 *= denominator1;
    // add them:
    Polynomial<std::complex<double> > numerator = numerator1;
    numerator += numerator2;
    // set the feed forward coefficients from the new numerator:
    feedForward.resize(numerator.size());
    x.resize(numerator.size());
    for (int i = 0; i < feedForward.size(); i++) {
        feedForward[i] = numerator[i].real();
    }
    // set the feedback coefficients from the new denominator:
    feedBack.resize(denominator.size() - 1);
    y.resize(denominator.size() - 1);
    for (int i = 0; i < feedBack.size(); i++) {
        feedBack[i] = denominator[i + 1].real();
    }
    return *this;
}

IIRFilter& IIRFilter::operator*=(const IIRFilter &b)
{
    // multiply numerators and denominators:
    Polynomial<std::complex<double> > numerator = getNumeratorPolynomial();
    numerator *= b.getNumeratorPolynomial();
    Polynomial<std::complex<double> > denominator = getDenominatorPolynomial();
    denominator *= b.getDenominatorPolynomial();
    // set the feed forward coefficients from the new numerator:
    feedForward.resize(numerator.size());
    x.resize(numerator.size());
    for (int i = 0; i < feedForward.size(); i++) {
        feedForward[i] = numerator[i].real();
    }
    // set the feedback coefficients from the new denominator:
    feedBack.resize(denominator.size() - 1);
    y.resize(denominator.size() - 1);
    for (int i = 0; i < feedBack.size(); i++) {
        feedBack[i] = denominator[i + 1].real();
    }
    return *this;
}

Polynomial<std::complex<double> > IIRFilter::getNumeratorPolynomial() const
{
    Polynomial<std::complex<double> > numerator(feedForward.size() ? feedForward[0] : 0);
    for (int i = 1; i < feedForward.size(); i++) {
        numerator.push_back(feedForward[i]);
    }
    return numerator;
}

Polynomial<std::complex<double> > IIRFilter::getDenominatorPolynomial() const
{
    Polynomial<std::complex<double> > denominator(1);
    for (int i = 0; i < feedBack.size(); i++) {
        denominator.push_back(feedBack[i]);
    }
    return denominator;
}
