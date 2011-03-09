#ifndef IIRFILTER_H
#define IIRFILTER_H

#include <QVector>
#include <complex>
#include "audioprocessor.h"
#include "eventprocessor.h"
#include "frequencyresponse.h"
#include "polynomial.h"

class IirFilter : public AudioProcessor, public EventProcessor, public FrequencyResponse
{
public:
    IirFilter(int feedForwardCoefficients, int feedBackCoefficients, const QStringList &additionalInputPortNames = QStringList(), double sampleRate = 44100);
    IirFilter(const IirFilter &tocopy);

    // reimplemented from AudioProcessor:
    virtual void processAudio(const double *inputs, double *outputs, jack_nframes_t time);
    // reimplemented from EventProcessor:
    virtual bool processEvent(const RingBufferEvent *event, jack_nframes_t time);
    // reimplemented from FrequencyResponse:
    virtual double getSquaredAmplitudeResponse(double hertz);

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
