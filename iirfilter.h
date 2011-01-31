#ifndef IIRFILTER_H
#define IIRFILTER_H

#include <QVector>
#include <complex>
#include "frequencyresponse.h"
#include "polynomial.h"

class IIRFilter : public FrequencyResponse
{
public:
    IIRFilter(double sampleRate);
    IIRFilter(int feedForwardCoefficients, int feedBackCoefficients, double sampleRate);

    void debug();

    double getSampleRate() const;
    double getFrequencyInRadians(double frequencyInHertz) const;
    double filter(double x);
    void reset();
    double getSquaredAmplitudeResponse(double hertz);

    void addFeedForwardCoefficient(double c);
    void addFeedBackCoefficient(double c);

    int getFeedForwardCoefficientCount() const;
    int getFeedBackCoefficientCount() const;

    void invert();

    static int computeBinomialCoefficient(int n, int k);

protected:
    void setFeedForwardCoefficient(int index, double c);
    void setFeedBackCoefficient(int index, double c);
    double getFeedForwardCoefficient(int index) const;
    double getFeedBackCoefficient(int index) const;

private:
    double sampleRate;
    QVector<double> feedForward, feedBack, x, y;

    Polynomial<std::complex<double> > getNumeratorPolynomial() const;
    Polynomial<std::complex<double> > getDenominatorPolynomial() const;
};

#endif // IIRFILTER_H
