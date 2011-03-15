#ifndef INTEGRALOSCILLATOR_H
#define INTEGRALOSCILLATOR_H

#include "oscillator.h"
#include "interpolatorprocessor.h"
#include "polynomialinterpolator.h"
#include <QQueue>

class PolynomialOscillator : public Oscillator, public InterpolatorProcessor
{
public:
    PolynomialOscillator(int nrOfIntegrations, double sampleRate = 44100, const QStringList &additionalInputPortNames = QStringList());

    PolynomialInterpolator * getPolynomialInterpolator();
    void setPolynomialInterpolator(const PolynomialInterpolator &interpolator);

    // Reimplemented from Oscillator:
    virtual bool processEvent(const RingBufferEvent *event, jack_nframes_t time);
protected:
    double valueAtPhase(double normalizedPhase);
    double differentiate(int order);
    void changeControlPoints(const QVector<double> &xx, const QVector<double> &yy);
private:
    int nrOfIntegrations;
    QVector<PolynomialInterpolator> integrals;
    QVector<double> previousIntegralValues;
    QQueue<double> previousPhases, previousPhaseDifferences;

    void computeIntegrals();
};

#endif // INTEGRALOSCILLATOR_H
