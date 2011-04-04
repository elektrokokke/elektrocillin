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

#include "iirbutterworthfilter.h"
#include <cmath>

IirButterworthFilter::IirButterworthFilter(double cutoffFrequencyInHertz, Type type_) :
    IirFilter(3, 2, QStringList()),
    type(type_)
{
    setCutoffFrequency(cutoffFrequencyInHertz);
}

void IirButterworthFilter::setCutoffFrequency(double cutoffFrequencyInHertz, Type type)
{
    this->cutoffFrequency = cutoffFrequencyInHertz;
    this->type = type;
    double radians = convertHertzToRadians(type == LOW_PASS ? cutoffFrequency : getSampleRate() * 0.5 - cutoffFrequencyInHertz);
    double c = cos(radians * 0.5) / sin(radians * 0.5);
    double factor = 1 / (c*c + c*sqrt(2.0) + 1.0);
    getFeedBackCoefficients()[0] = (-c*c*2.0 + 2.0) * factor;
    getFeedBackCoefficients()[1] = (c*c - c*sqrt(2.0) + 1.0) * factor;
    getFeedForwardCoefficients()[0] = factor;
    getFeedForwardCoefficients()[1] = 2 * factor;
    getFeedForwardCoefficients()[2] = factor;
    if (type == HIGH_PASS) {
        invert();
    }
}

void IirButterworthFilter::setCutoffFrequency(double cutoffFrequencyInHertz)
{
    setCutoffFrequency(cutoffFrequencyInHertz, getType());
}

void IirButterworthFilter::setType(Type type)
{
    setCutoffFrequency(getCutoffFrequency(), type);
}

double IirButterworthFilter::getCutoffFrequency() const
{
    return cutoffFrequency;
}

IirButterworthFilter::Type IirButterworthFilter::getType() const
{
    return type;
}
