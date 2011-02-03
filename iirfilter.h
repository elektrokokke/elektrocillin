#ifndef IIRFILTER_H
#define IIRFILTER_H

#include <QVector>
#include <complex>
#include "midiprocessor.h"
#include "frequencyresponse.h"
#include "polynomial.h"

class IirFilter : public MidiProcessor, public FrequencyResponse
{
public:
    IirFilter(int feedForwardCoefficients, int feedBackCoefficients, int nrOfInputs = 1, double sampleRate = 44100);

    // reimplemented from AudioProcessor:
    virtual void processAudio(const double *inputs, double *outputs, jack_nframes_t time);
    // reimplenented from FrequencyResponse:
    double getSquaredAmplitudeResponse(double hertz);

    QVector<double> & getFeedForwardCoefficients();
    QVector<double> & getFeedBackCoefficients();

    QString toString() const;

    void reset();

    // filter arithmetic:
    void invert();
    // add another IIRFilter (which means parallel operation):
    IirFilter& operator+=(const IirFilter &b);
    // multiply with another IIRFilter (which means serial operation):
    IirFilter& operator*=(const IirFilter &b);

    static int computeBinomialCoefficient(int n, int k);
private:
    QVector<double> feedForward, feedBack, x, y;

    Polynomial<std::complex<double> > getNumeratorPolynomial() const;
    Polynomial<std::complex<double> > getDenominatorPolynomial() const;
};

#endif // IIRFILTER_H
