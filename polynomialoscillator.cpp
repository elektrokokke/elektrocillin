/*
    Copyright 2011 Arne Jacobs <jarne@jarne.de>

    This file is part of elektrocillin.

    Elektrocillin is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "polynomialoscillator.h"

PolynomialOscillator::PolynomialOscillator(int nrOfIntegrations_, const QStringList &additionalInputPortNames) :
    Oscillator(additionalInputPortNames),
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

PolynomialInterpolator * PolynomialOscillator::getPolynomialInterpolator()
{
    return &integrals.first();
}

void PolynomialOscillator::setPolynomialInterpolator(const PolynomialInterpolator &interpolator)
{
    integrals[0] = interpolator;
    computeIntegrals();
}

bool PolynomialOscillator::processEvent(const RingBufferEvent *event, jack_nframes_t time)
{
    if (const InterpolatorProcessor::InterpolatorEvent *event_ = dynamic_cast<const InterpolatorProcessor::InterpolatorEvent*>(event)) {
        InterpolatorProcessor::processInterpolatorEvent(&integrals.first(), event_);
        computeIntegrals();
        return true;
    } else {
        return false;
    }
}

double PolynomialOscillator::valueAtPhase(double phase)
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

double PolynomialOscillator::differentiate(int order)
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

void PolynomialOscillator::changeControlPoints(const QVector<double> &xx, const QVector<double> &yy)
{
    integrals.first().changeControlPoints(xx, yy);
    computeIntegrals();
}

void PolynomialOscillator::computeIntegrals()
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
