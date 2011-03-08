#ifndef INTEGRALOSCILLATOR_H
#define INTEGRALOSCILLATOR_H

#include "oscillator.h"
#include "polynomialinterpolator.h"

class IntegralOscillator : public Oscillator
{
public:
    IntegralOscillator(double frequencyModulationIntensity = 2.0/12.0, double sampleRate = 44100, const QStringList &additionalInputPortNames = QStringList());

    PolynomialInterpolator * getPolynomialInterpolator();
    void setPolynomialInterpolator(const PolynomialInterpolator &interpolator);

    // Reimplemented from Oscillator:
    virtual bool processEvent(const RingBufferEvent *event, jack_nframes_t time);
protected:
    double valueAtPhase(double phase);

private:
    QVector<PolynomialInterpolator> integrals;
};

#endif // INTEGRALOSCILLATOR_H
