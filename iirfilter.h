#ifndef IIRFILTER_H
#define IIRFILTER_H

#include <QVector>
#include <complex>
#include "sampled.h"
#include "frequencyresponse.h"
#include "polynomial.h"

class IIRFilter : public Sampled, public FrequencyResponse
{
public:
    IIRFilter(double sampleRate = 44100);
    IIRFilter(int feedForwardCoefficients, int feedBackCoefficients, double sampleRate = 44100);

    // reimplemented from Sampled:
    void process(const double *inputs, double *outputs);
    // reimplenented from FrequencyResponse:
    double getSquaredAmplitudeResponse(double hertz);

    void addFeedForwardCoefficient(double c);
    void addFeedBackCoefficient(double c);

    int getFeedForwardCoefficientCount() const;
    int getFeedBackCoefficientCount() const;

    QString toString() const;

    void reset();

    // filter arithmetic:
    void invert();
    // add another IIRFilter (which means parallel operation):
    IIRFilter& operator+=(const IIRFilter &b);
    // multiply with another IIRFilter (which means serial operation):
    IIRFilter& operator*=(const IIRFilter &b);

    static int computeBinomialCoefficient(int n, int k);
protected:
    void setFeedForwardCoefficient(int index, double c);
    void setFeedBackCoefficient(int index, double c);
    double getFeedForwardCoefficient(int index) const;
    double getFeedBackCoefficient(int index) const;

private:
    QVector<double> feedForward, feedBack, x, y;

    Polynomial<std::complex<double> > getNumeratorPolynomial() const;
    Polynomial<std::complex<double> > getDenominatorPolynomial() const;
};

#endif // IIRFILTER_H
