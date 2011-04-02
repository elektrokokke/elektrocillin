#ifndef IIRFILTER_H
#define IIRFILTER_H

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

#include <QVector>
#include <complex>
#include "audioprocessor.h"
#include "eventprocessor.h"
#include "frequencyresponse.h"
#include "polynomial.h"

class IirFilter : public AudioProcessor, public FrequencyResponse
{
public:
    IirFilter(int feedForwardCoefficients, int feedBackCoefficients, const QStringList &additionalInputPortNames = QStringList(), double sampleRate = 44100);
    IirFilter(const IirFilter &tocopy);

    // reimplemented from AudioProcessor:
    virtual void processAudio(const double *inputs, double *outputs, jack_nframes_t time);
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
