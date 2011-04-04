/*
    Copyright 2011 Arne Jacobs <jarne@jarne.de>

    This file is part of elektrocillin.

    Elektrocillin is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "iirfilter.h"
#include <QDebug>

IirFilter::IirFilter(int feedForwardCoefficients, int feedBackCoefficients, const QStringList &additionalInputPortNames, double sampleRate) :
    AudioProcessor(QStringList("Audio in") + additionalInputPortNames, QStringList("Audio out"), sampleRate),
    feedForward(feedForwardCoefficients),
    feedBack(feedBackCoefficients),
    x(feedForwardCoefficients),
    y(feedBackCoefficients)
{
    reset();
}

IirFilter::IirFilter(const IirFilter &tocopy) :
    AudioProcessor(tocopy),
    feedForward(tocopy.feedForward),
    feedBack(tocopy.feedBack),
    x(tocopy.x),
    y(tocopy.y)
{
}

void IirFilter::processAudio(const double *inputs, double *outputs, jack_nframes_t)
{
    if (x.size()) {
        x[0] = inputs[0];
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
        outputs[0] = y[0] = result;
    } else {
        outputs[0] = 0.0;
    }
}

double IirFilter::getSquaredAmplitudeResponse(double hertz)
{
    std::complex<double> z_inv = 1.0 / std::exp(std::complex<double>(0.0, convertHertzToRadians(hertz)));
    Polynomial<std::complex<double> > numerator = getNumeratorPolynomial();
    Polynomial<std::complex<double> > denominator = getDenominatorPolynomial();
    return std::norm(numerator.evaluate(z_inv) / denominator.evaluate(z_inv));
}

QVector<double> & IirFilter::getFeedForwardCoefficients()
{
    return feedForward;
}

QVector<double> & IirFilter::getFeedBackCoefficients()
{
    return feedBack;
}

QString IirFilter::toString() const
{
    return polynomialToString(getNumeratorPolynomial()) + " / " + polynomialToString(getDenominatorPolynomial());
}

void IirFilter::reset()
{
    x.fill(0.0);
    y.fill(0.0);
}

void IirFilter::invert()
{
    // negate all coefficients with odd exponent:
    for (int i = 1; i < feedForward.size(); i += 2) {
        feedForward[i] = -feedForward[i];
    }
    for (int i = 0; i < feedBack.size(); i += 2) {
        feedBack[i] = -feedBack[i];
    }
}

IirFilter& IirFilter::operator+=(const IirFilter &b)
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

IirFilter& IirFilter::operator*=(const IirFilter &b)
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

int IirFilter::computeBinomialCoefficient(int n, int k)
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

Polynomial<std::complex<double> > IirFilter::getNumeratorPolynomial() const
{
    Polynomial<std::complex<double> > numerator(feedForward.size() ? feedForward[0] : 0);
    for (int i = 1; i < feedForward.size(); i++) {
        numerator.push_back(feedForward[i]);
    }
    return numerator;
}

Polynomial<std::complex<double> > IirFilter::getDenominatorPolynomial() const
{
    Polynomial<std::complex<double> > denominator(1);
    for (int i = 0; i < feedBack.size(); i++) {
        denominator.push_back(feedBack[i]);
    }
    return denominator;
}
