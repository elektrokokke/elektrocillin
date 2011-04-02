#ifndef INTEGRALOSCILLATOR_H
#define INTEGRALOSCILLATOR_H

/*
    Copyright 2011 Arne Jacobs

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
