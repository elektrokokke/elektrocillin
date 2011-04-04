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

#include "butterworth2polefilter.h"
#include <QDebug>

Butterworth2PoleFilter::Butterworth2PoleFilter(double normalizedCutoffFrequency)
{
    double frequencyInRadians = M_PI * normalizedCutoffFrequency;
    double c = cos(frequencyInRadians * 0.5) / sin(frequencyInRadians * 0.5);
    // butterworth 2-pole-filter coefficients:
    std::complex<double> feedback[] = { c*c + c*sqrt(2.0) + 1.0, -c*c*2.0 + 2.0, c*c - c*sqrt(2.0) + 1.0 };
    std::complex<double> feedforward[] = { 1.0, 2.0, 1.0 };
    //qDebug() << "numerator";
    for (size_t i = 0; i < 3; i++) {
        //qDebug() << feedforward[i].real() << feedforward[i].imag();
    }
    //qDebug() << "denominator";
    for (size_t i = 0; i < 3; i++) {
        //qDebug() << feedback[i].real() << feedback[i].imag();
    }
    // setup the z-plane filter:
    addPole(std::complex<double>((-feedback[1] - std::sqrt(feedback[1] * feedback[1] - 4.0 * feedback[2] * feedback[0])) / (2.0 * feedback[0])));
    addPole(std::complex<double>((-feedback[1] + std::sqrt(feedback[1] * feedback[1] - 4.0 * feedback[2] * feedback[0])) / (2.0 * feedback[0])));
    addZero(std::complex<double>((-feedforward[1] - std::sqrt(feedforward[1] * feedforward[1] - 4.0 * feedforward[2] * feedforward[0])) / (2.0 * feedforward[0])));
    addZero(std::complex<double>((-feedforward[1] + std::sqrt(feedforward[1] * feedforward[1] - 4.0 * feedforward[2] * feedforward[0])) / (2.0 * feedforward[0])));
    computeCoefficients();
}
