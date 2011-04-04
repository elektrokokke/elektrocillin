#ifndef CUBICSPLINEINTERPOLATOR_H
#define CUBICSPLINEINTERPOLATOR_H

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

#include "interpolator.h"

class CubicSplineInterpolator : public Interpolator
{
public:
    /**
      Constructor for splines with given first derivatives at the start and end.
      */
    CubicSplineInterpolator(const QVector<double> &xx, const QVector<double> &yy, double yp1, double ypn);
    /**
      Constructor for "natural" splines.
      */
    CubicSplineInterpolator(const QVector<double> &xx, const QVector<double> &yy);
    /**
      Constructor with given second derivatives.
      */
    CubicSplineInterpolator(const QVector<double> &xx, const QVector<double> &yy, const QVector<double> &y2);

    virtual void save(QDataStream &stream) const;
    virtual void load(QDataStream &stream);

    const QVector<double> & getY() const;
    const QVector<double> & getY2() const;

    double interpolate(int jlo, double x);

    virtual void changeControlPoints(const QVector<double> &xx, const QVector<double> &yy);
    virtual void changeControlPoint(int index, double x, double y);
    virtual void addControlPoints(bool scaleX, bool scaleY, bool addAtStart, bool addAtEnd);
    virtual void deleteControlPoints(bool scaleX, bool scaleY, bool deleteAtStart, bool deleteAtEnd);
private:
    QVector<double> y2;

    void sety2(const QVector<double> &xx, const QVector<double> &yy, double yp1, double ypn);
    void sety2NaturalSpline(const QVector<double> &xx, const QVector<double> &yy);
};

#endif // CUBICSPLINEINTERPOLATOR_H
