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

#include "linearinterpolator.h"
#include <QDebug>

LinearInterpolator::LinearInterpolator() :
    Interpolator(QVector<double>(), QVector<double>(), 2)
{
}

LinearInterpolator::LinearInterpolator(const QVector<double> &xx, const QVector<double> &yy) :
    Interpolator(xx, yy, 2)
{
}

/**
  Comments from "Numerical Recipes"

  Piecewise linear interpolation object. Construct with x and y vectors,
  then call interpolate for interpolated values.
  */
double LinearInterpolator::interpolate(int j, double x)
{
    Q_ASSERT(xx.size() >= 2);
    if (j < 0) {
        j = 0;
    }
    if (j >= xx.size() - 1) {
        if (xx[xx.size() - 1] == xx[xx.size() - 2]) {
            return yy.last();
        } else {
            j = xx.size() - 2;
        }
    }
    if (xx[j] == xx[j + 1]) {   // Table is defective, but we can recover.
        return yy[j];
    } else {
        return yy[j] + ((x - xx[j]) / (xx[j + 1] - xx[j])) * (yy[j + 1] - yy[j]);
    }
}

