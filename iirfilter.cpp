#include "iirfilter.h"
#include <complex>
#include <QDebug>

IIRFilter::IIRFilter(double sampleRate_) :
    sampleRate(sampleRate_)
{
}

IIRFilter::IIRFilter(int feedForwardCoefficients, int feedBackCoefficients, double sampleRate_) :
    sampleRate(sampleRate_),
    feedForward(feedForwardCoefficients),
    feedBack(feedBackCoefficients)
{
}

void IIRFilter::debug()
{
    qDebug() << "feedForward";
    for (int i = 0; i < feedForward.size(); i++) {
        qDebug() << feedForward[i];
    }
    qDebug() << "feedBack";
    for (int i = 0; i < feedBack.size(); i++) {
        qDebug() << feedBack[i];
    }
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
        double nominator = 0.0;
        for (int i = 0; i < x.size(); i++) {
            nominator += x[i] * feedForward[i];
        }
        double denumerator = 1.0;
        for (int i = 0; i < y.size(); i++) {
            denumerator += y[i] * feedBack[i];
        }
        double result = nominator / denumerator;
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
        return 0.0;
    }
}

void IIRFilter::reset()
{
    x.fill(0.0);
    y.fill(0.0);
}

double IIRFilter::getSquaredAmplitudeResponse(double hertz)
{
    std::complex<double> z = std::exp(std::complex<double>(0.0, getFrequencyInRadians(hertz)));
    std::complex<double> numerator(0);
    for (int i = 0; i < feedForward.size(); i++) {
        numerator += feedForward[i] * std::pow(z, -i);
    }
    std::complex<double> denominator(1);
    for (int i = 0; i < feedBack.size(); i++) {
        denominator += feedBack[i] * std::pow(z, -(i + 1));
    }
    return std::norm(numerator / denominator);
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
