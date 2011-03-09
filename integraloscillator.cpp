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

bool IntegralOscillator::processEvent(const RingBufferEvent *event, jack_nframes_t time)
{
    if (const InterpolatorProcessor::InterpolatorEvent *event_ = dynamic_cast<const InterpolatorProcessor::InterpolatorEvent*>(event)) {
        InterpolatorProcessor::processInterpolatorEvent(&integrals.first(), event_);
        return true;
    } else {
        return Oscillator::processEvent(event, time);
    }
}

double IntegralOscillator::valueAtPhase(double phase)
{
    // TODO
    return 0;
}
