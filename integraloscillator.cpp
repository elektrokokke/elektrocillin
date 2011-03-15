#include "integraloscillator.h"

IntegralOscillator::IntegralOscillator(int nrOfIntegrations_, double sampleRate, const QStringList &additionalInputPortNames) :
    Oscillator(sampleRate, additionalInputPortNames),
    nrOfIntegrations(nrOfIntegrations_),
    integrals(nrOfIntegrations + 1),
    previousIntegralValues(nrOfIntegrations, 0)
{
    QVector<double> xx, yy;
    xx.append(0);
    yy.append(-1);
    xx.append(1);
    yy.append(1);
    integrals[0] = PolynomialInterpolator(xx, yy);
    for (int i = 0; i < nrOfIntegrations; i++) {
        previousPhases.enqueue(0);
        previousPhaseDifferences.enqueue(1);
    }
    previousPhases.enqueue(0);
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

double IntegralOscillator::valueAtPhase(double phase)
{
    double phaseDifference = phase - previousPhases.back();
    if (phaseDifference <= 0) {
        phaseDifference += 1;
    }
    previousPhases.enqueue(phase);
    previousPhases.dequeue();
    previousPhaseDifferences.enqueue(phaseDifference);
    previousPhaseDifferences.dequeue();
    double value = differentiate(nrOfIntegrations);
    return value;
}

double IntegralOscillator::differentiate(int order)
{
    // evaluate the top integral at the given phase:
    double value = integrals.back().evaluate(previousPhases[order]);
    // differentiate "order" times:
    double phaseDifferencesSum = 0;
    double factor = 0;
    for (int i = 0; i < order; i++) {
        double previousValue = previousIntegralValues[i];
        previousIntegralValues[i] = value;
        phaseDifferencesSum += previousPhaseDifferences[order - i - 1];
        factor++;
        value = (value - previousValue) * factor / phaseDifferencesSum;
    }
    return value;
}

void IntegralOscillator::computeIntegrals()
{
    for (int i = 0; i < nrOfIntegrations; i++) {
        // integrate the previous piece-wise polynomial:
        integrals[i + 1].integrate(integrals[i]);
        // smoothen the result (match start and end points):
        integrals[i + 1].smoothen();
    }
    for (int i = 0; i < nrOfIntegrations; i++) {
        differentiate(i + 1);
    }
}
