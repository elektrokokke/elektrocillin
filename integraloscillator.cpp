#include "integraloscillator.h"

IntegralOscillator::IntegralOscillator(int nrOfIntegrations_, double frequencyModulationIntensity, double sampleRate, const QStringList &additionalInputPortNames) :
    Oscillator(frequencyModulationIntensity, sampleRate, additionalInputPortNames),
    nrOfIntegrations(nrOfIntegrations_),
    integrals(nrOfIntegrations + 1),
    previousIntegralValues(nrOfIntegrations, 0),
    previousPhase(0)
{
    QVector<double> xx, yy;
    xx.append(0);
    yy.append(-1);
    xx.append(1);
    yy.append(1);
    integrals[0] = PolynomialInterpolator(xx, yy);
    computeIntegrals();
}

PolynomialInterpolator * IntegralOscillator::getPolynomialInterpolator()
{
    return &integrals.first();
}

void IntegralOscillator::setPolynomialInterpolator(const PolynomialInterpolator &interpolator)
{
    integrals[0] = interpolator;
    computeIntegrals();
}

bool IntegralOscillator::processEvent(const RingBufferEvent *event, jack_nframes_t time)
{
    if (const InterpolatorProcessor::InterpolatorEvent *event_ = dynamic_cast<const InterpolatorProcessor::InterpolatorEvent*>(event)) {
        InterpolatorProcessor::processInterpolatorEvent(&integrals.first(), event_);
        computeIntegrals();
        return true;
    } else {
        return Oscillator::processEvent(event, time);
    }
}

double IntegralOscillator::valueAtPhase(double normalizedPhase)
{
    double phaseDifference = normalizedPhase - previousPhase;
    if (phaseDifference <= 0) {
        phaseDifference += 1;
    }
    previousPhase = normalizedPhase;
    // evaluate the top integral at the current phase:
    double value = integrals.back().evaluate(normalizedPhase);
    // differentiate as many times as we have integrated:
    for (int i = 0; i < nrOfIntegrations; i++) {
        double previousValue = previousIntegralValues[nrOfIntegrations - i - 1];
        previousIntegralValues[nrOfIntegrations - i - 1] = value;
        value = (value - previousValue) / phaseDifference;
    }
    return value;
}

void IntegralOscillator::computeIntegrals()
{
    for (int i = 1; i < integrals.size(); i++) {
        // integrate the previous piece-wise polynomial:
        integrals[i].integrate(integrals[i - 1]);
        // smoothen the result (match start and end points):
        integrals[i].smoothen();
    }
}
