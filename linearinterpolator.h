#ifndef LINEARINTERPOLATOR_H
#define LINEARINTERPOLATOR_H

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

#include "interpolator.h"
#include <QVector>
#include <QDataStream>

class LinearInterpolator : public Interpolator
{
public:
    LinearInterpolator();
    LinearInterpolator(const QVector<double> &xx, const QVector<double> &yy);

    double interpolate(int jlo, double x);
};

#endif // LINEARINTERPOLATOR_H
