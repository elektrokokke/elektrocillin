#include "integraloscillator.h"

IntegralOscillator::IntegralOscillator(double frequencyModulationIntensity, double sampleRate, const QStringList &additionalInputPortNames) :
    Oscillator(frequencyModulationIntensity, sampleRate, additionalInputPortNames)
{
    QVector<double> xx, yy;
    xx.append(0);
    yy.append(-1);
    xx.append(1);
    yy.append(1);
    integrals.append(PolynomialInterpolator(xx, yy));
}

PolynomialInterpolator * IntegralOscillator::getPolynomialInterpolator()
{
    return &integrals.first();
}

void IntegralOscillator::setPolynomialInterpolator(const PolynomialInterpolator &interpolator)
{
    integrals[0] = interpolator;
}

void IntegralOscillator::processEvent(const Interpolator::ChangeControlPointEvent *event, jack_nframes_t)
{
    integrals.first().processEvent(event);
}

void IntegralOscillator::processEvent(const Interpolator::ChangeAllControlPointsEvent *event, jack_nframes_t)
{
    integrals.first().processEvent(event);
}

double IntegralOscillator::valueAtPhase(double phase)
{
    // TODO
    return 0;
}
