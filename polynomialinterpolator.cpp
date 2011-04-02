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

#include "polynomialinterpolator.h"

PolynomialInterpolator::PolynomialInterpolator() :
    Interpolator(QVector<double>(), QVector<double>(), 2)
{
}

PolynomialInterpolator::PolynomialInterpolator(const QVector<double> &xx, const QVector<double> &yy) :
    Interpolator(xx, yy, 2)
{
    initialize(xx, yy);
}

void PolynomialInterpolator::save(QDataStream &stream) const
{
    Interpolator::save(stream);
    for (int i = 0; i < polynomials.size(); i++) {
        int polynomialSize = polynomials[i].size();
        stream << polynomialSize;
        for (int j = 0; j < polynomialSize; j++) {
            stream << polynomials[i][j];
        }
    }
}

void PolynomialInterpolator::load(QDataStream &stream)
{
    Interpolator::load(stream);
    polynomials.resize(xx.size() - 1);
    for (int i = 0; i < polynomials.size(); i++) {
        int polynomialSize;
        stream >> polynomialSize;
        polynomials[i].resize(polynomialSize);
        for (int j = 0; j < polynomialSize; j++) {
            stream >> polynomials[i][j];
        }
    }
}

void PolynomialInterpolator::integrate(const PolynomialInterpolator &polynomialInterpolator)
{
    xx = polynomialInterpolator.getX();
    polynomials.resize(polynomialInterpolator.polynomials.size());
    // first integrate all segments independently:
    for (int i = 0; i < polynomials.size(); i++) {
        polynomials[i].integrate(polynomialInterpolator.polynomials[i]);
    }
    // now make their ends match:
    for (int i = 1; i < polynomials.size(); i++) {
        polynomials[i].adjustConstantToIntersect(xx[i], polynomials[i - 1].evaluate(xx[i]));
    }
}

void PolynomialInterpolator::smoothen()
{
    double x1 = xx.first();
    double x2 = xx.last();
    if (x1 != x2) {
        // get the value at the start of the first segment:
        double startValue = polynomials.first().evaluate(x1);
        // get the value at the end of the last segment:
        double endValue = polynomials.last().evaluate(x2);
        // the line to compensate should be zero at the start and (startValue - endValue) at the end:
        double y1 = 0;
        double y2 = startValue - endValue;
        // compute the coefficients of the line between both points:
        double a = (y2 - y1) / (x2 - x1);
        double b = y1 - a * x1;
        Polynomial<double> lineToAdd(b, a);
        for (int i = 0; i < polynomials.size(); i++) {
            polynomials[i] += lineToAdd;
        }
    }
}

double PolynomialInterpolator::interpolate(int j, double x)
{
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
    return polynomials[j].evaluate(x);
}

void PolynomialInterpolator::changeControlPoints(const QVector<double> &xx, const QVector<double> &yy)
{
    Interpolator::changeControlPoints(xx, yy);
    initialize(xx, yy);
}

void PolynomialInterpolator::changeControlPoint(int index, double x, double y)
{
    Interpolator::changeControlPoint(index, x, y);
    if (index > 0) {
        initializePolynomial(index - 1, xx[index - 1], yy[index - 1], xx[index], yy[index]);
    }
    if (index < xx.size() - 1) {
        initializePolynomial(index, xx[index], yy[index], xx[index + 1], yy[index + 1]);
    }
}

void PolynomialInterpolator::addControlPoints(bool scaleX, bool scaleY, bool addAtStart, bool addAtEnd)
{
    Interpolator::addControlPoints(scaleX, scaleY, addAtStart, addAtEnd);
    initialize(getX(), getY());
}

void PolynomialInterpolator::deleteControlPoints(bool scaleX, bool scaleY, bool deleteAtStart, bool deleteAtEnd)
{
    Interpolator::deleteControlPoints(scaleX, scaleY, deleteAtStart, deleteAtEnd);
    initialize(getX(), getY());
}

void PolynomialInterpolator::initialize(const QVector<double> &xx, const QVector<double> &yy)
{
    polynomials.resize(xx.size() - 1);
    this->xx = xx;
    for (int i = 0; i < xx.size() - 1; i++) {
        initializePolynomial(i, xx[i], yy[i], xx[i + 1], yy[i + 1]);
    }
}

void PolynomialInterpolator::initializePolynomial(int i, double x1, double y1, double x2, double y2)
{
    // initialize the single-degree polynomials based on the given control points:
    if (x1 == x2) {
        polynomials[i].resize(1);
        polynomials[i].at(0) = y1;
    } else {
        double a = (y2 - y1) / (x2 - x1);
        double b = y1 - a * x1;
        polynomials[i].resize(2);
        polynomials[i].at(0) = b;
        polynomials[i].at(1) = a;
    }
}
