#ifndef LINEARINTEGRALINTERPOLATOR_H
#define LINEARINTEGRALINTERPOLATOR_H

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

#include "linearinterpolator.h"

/**
  Represents the indefinite integral of a piecewise linear function,
  whose parameters are given via a LinearInterpolator object.
  */

class LinearIntegralInterpolator : public Interpolator
{
public:
    LinearIntegralInterpolator(const LinearInterpolator &linearInterpolator);

    virtual void save(QDataStream &stream) const;
    virtual void load(QDataStream &stream);

    virtual double interpolate(int jlo, double x);

private:
    QVector<double> a, b, c;
};

#endif // LINEARINTEGRALINTERPOLATOR_H
