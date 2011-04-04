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

#include "logarithmicinterpolator.h"
#include <QDebug>
#include <cmath>

LogarithmicInterpolator::LogarithmicInterpolator(double base_) :
    Interpolator(QVector<double>(), QVector<double>(), 2),
    base(base_)
{
}

LogarithmicInterpolator::LogarithmicInterpolator(const QVector<double> &xx, const QVector<double> &yy, double base_) :
    Interpolator(xx, yy, 2),
    base(base_)
{
}

void LogarithmicInterpolator::save(QDataStream &stream) const
{
    Interpolator::save(stream);
    stream << base;
}

void LogarithmicInterpolator::load(QDataStream &stream)
{
    Interpolator::load(stream);
    stream >> base;
}

double LogarithmicInterpolator::interpolate(int j, double x)
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
    if (xx[j] == xx[j + 1]) {
        return yy[j];
    } else if (x == xx[j]) {
        return yy[j];
    } else if (x == xx[j + 1]) {
        return yy[j + 1];
    } else {
        double weight2 = (x - xx[j]) / (xx[j + 1] - xx[j]);
        if (base != 1) {
            weight2 = (1.0 - pow(base, weight2)) / (1.0 - base);
        }
        double weight1 = 1.0 - weight2;
        return yy[j] * weight1 + yy[j + 1] * weight2;
    }
}
