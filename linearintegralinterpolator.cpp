/*
    Copyright 2011 Arne Jacobs <jarne@jarne.de>

    This file is part of elektrocillin.

    Elektrocillin is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Elektrocillin is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Elektrocillin.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "linearintegralinterpolator.h"

LinearIntegralInterpolator::LinearIntegralInterpolator(const LinearInterpolator &linear) :
    Interpolator(linear.getX(), linear.getY(), linear.getM()),
    a(linear.getX().size() - 1),
    b(linear.getX().size() - 1),
    c(linear.getX().size() - 1)
{
    // compute coefficients of the quadratic functions which make up the integral:
    double previousIntegralValue = 0;
    for (int i = 0; i < a.size(); i++) {
        if (getX()[i] == getX()[i + 1]) {
            // discontinuity in the function, insert "empty" quadratic:
            a[i] = b[i] = 0;
            c[i] = previousIntegralValue;
        } else {
            // f'(x) = 2 * a * x + b
            a[i] = 0.5 * (linear.getY()[i] - linear.getY()[i + 1]) / (getX()[i] - getX()[i + 1]);
            b[i] = linear.getY()[i] - 2.0 * a[i] * linear.getX()[i];
            // f(x) = a * x^2 + b * x + c
            c[i] = previousIntegralValue - a[i] * getX()[i] * getX()[i] - b[i] * getX()[i];
            previousIntegralValue = a[i] * getX()[i + 1] * getX()[i + 1] + b[i] * getX()[i + 1] + c[i];
        }
    }
}

void LinearIntegralInterpolator::save(QDataStream &stream) const
{
    Interpolator::save(stream);
    stream << a << b << c;
}

void LinearIntegralInterpolator::load(QDataStream &stream)
{
    Interpolator::load(stream);
    stream >> a >> b >> c;
}

double LinearIntegralInterpolator::interpolate(int jlo, double x)
{
    return a[jlo] * x * x + b[jlo] * x + c[jlo];
}
