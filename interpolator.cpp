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
#include <cmath>

Interpolator::~Interpolator()
{
}

void Interpolator::setControlPointName(int controlPointIndex, const QString &name)
{
    if (name.isNull()) {
        names.remove(controlPointIndex);
    } else {
        Q_ASSERT((controlPointIndex >= 0) && (controlPointIndex < xx.size()));
        names[controlPointIndex] = name;
    }
}

QString Interpolator::getControlPointName(int controlPointIndex) const
{
    return names.value(controlPointIndex);
}

double Interpolator::evaluate(double x, int *index)
{
    int jlo = cor ? hunt(x) : locate(x);
    if (index) {
        *index = jlo;
    }
    return interpolate(jlo, x);
}

void Interpolator::reset()
{
    jsav = 0;
    cor = 1;
}

const QVector<double> & Interpolator::getX() const
{
    return xx;
}

const QVector<double> & Interpolator::getY() const
{
    return yy;
}

int Interpolator::getM() const {
    return mm;
}

void Interpolator::save(QDataStream &stream) const
{
    stream << xx << yy;
}

void Interpolator::load(QDataStream &stream)
{
    stream >> xx >> yy;
}

void Interpolator::changeControlPoints(const QVector<double> &xx, const QVector<double> &yy)
{
    // note: no check is done here wether the given points adhere to all constraints
    // (e.g., the monotonicity constraints)
    this->xx = xx;
    this->yy = yy;
    controlPointsChanged();
}

int Interpolator::getNrOfControlPoints()
{
    return xx.size();
}

QPointF Interpolator::getControlPoint(int index)
{
    Q_ASSERT((index >= 0) && (index < getNrOfControlPoints()));
    return QPointF(xx[index], yy[index]);
}

void Interpolator::changeControlPoint(int index, double x, double y)
{
    Q_ASSERT((index >= 0) && (index < xx.size()));
    if (index == 0) {
        // fix the start point if desired:
        if (xIsStatic.first) {
            x = xx.first();
        }
        if (yIsStatic.first) {
            y = yy.first();
        }
    } else {
        // make sure that all x values increase monotonically:
        if (isStrictlyMonotonic && (x <= xx[index - 1])) {
            return;
        } else if (x < xx[index - 1]) {
            x = xx[index - 1];
        }
    }
    if (index == xx.size() - 1) {
        // fix the end point if desired:
        if (xIsStatic.second) {
            x = xx.last();
        }
        if (yIsStatic.second) {
            y = yy.last();
        }
    } else {
        // make sure that all x values increase monotonically:
        if (isStrictlyMonotonic && (x >= xx[index + 1])) {
            return;
        } else if (x > xx[index + 1]) {
            x = xx[index + 1];
        }
    }
    // enforce the y range constraint:
    y = qMax(qMin(y, yMax), yMin);
    xx[index] = x;
    yy[index] = y;
    controlPointsChanged();
}

void Interpolator::addControlPoint(double x, double y)
{
    int index;
    evaluate(x, &index);
    // make sure the point lies after the first and before the last:
    if ((index >= 0) && (index < xx.size() - 1)) {
        // make sure the monotonicity constraints are met:
        if (isStrictlyMonotonic) {
            if ((x <= xx[index]) || ((index < xx.size()) && (x >= xx[index + 1]))) {
                return;
            }
        } else if (x < xx[index]) {
            x = xx[index];
        } else if ((index < xx.size()) && (x > xx[index + 1])) {
            x = xx[index + 1];
        }
        // enforce the y range constraints:
        y = qMax(qMin(y, yMax), yMin);
        // insert a new control point:
        xx.insert(index + 1, x);
        yy.insert(index + 1, y);
        controlPointsChanged();
    }
}

void Interpolator::deleteControlPoint(int index)
{
    if ((index >= 0) && (index < xx.size()) && (xx.size() > 2)) {
        if (index == 0) {
            // fix the start point if desired:
            if (xIsStatic.first) {
                xx[1] = xx.first();
            }
            if (yIsStatic.first) {
                yy[1] = yy.first();
            }
        } else if (index == xx.size() - 1) {
            // fix the end point if desired:
            if (xIsStatic.second) {
                xx[xx.size() - 2] = xx.last();
            }
            if (yIsStatic.second) {
                yy[yy.size() - 2] = yy.last();
            }
        }
        // delete the control point in question:
        xx.remove(index);
        yy.remove(index);
        controlPointsChanged();
    }
}

void Interpolator::processInterpolatorEvent(const InterpolatorEvent *event)
{
    if (const ChangeControlPointEvent *event_ = dynamic_cast<const ChangeControlPointEvent*>(event)) {
        changeControlPoint(event_->index, event_->x, event_->y);
    } else if (const AddControlPointEvent *event_ = dynamic_cast<const AddControlPointEvent*>(event)) {
        addControlPoint(event_->x, event_->y);
    } else if (const DeleteControlPointEvent *event_ = dynamic_cast<const DeleteControlPointEvent*>(event)) {
        deleteControlPoint(event_->index);
    }
}


void Interpolator::setMonotonicity(bool isStrictlyMonotonic)
{
    this->isStrictlyMonotonic = isStrictlyMonotonic;
}

void Interpolator::setStartPointConstraints(bool xIsStatic, bool yIsStatic)
{
    this->xIsStatic.first = xIsStatic;
    this->yIsStatic.first = yIsStatic;
}

void Interpolator::setEndPointConstraints(bool xIsStatic, bool yIsStatic)
{
    this->xIsStatic.second = xIsStatic;
    this->yIsStatic.second = yIsStatic;
}

void Interpolator::setYRange(double yMin, double yMax)
{
    this->yMin = yMin;
    this->yMax = yMax;
}

Interpolator::Interpolator(const QVector<double> &xx_, const QVector<double> &yy_, int m_) :
    xx(xx_),
    yy(yy_),
    mm(m_),
    jsav(0),
    cor(0),
    isStrictlyMonotonic(false),
    xIsStatic(QPair<bool, bool>(true, true)),
    yIsStatic(QPair<bool, bool>(false, false)),
    yMin(-1),
    yMax(1)
{
    dj = std::max(1, (int)pow((double)xx.size(), 0.25));
    previousN = xx.size();
}

void Interpolator::controlPointsChanged()
{
}

/**
  Comments from "Numerical Recipes"

  Given a value x, return a value j such that x is (insofar as possible) centered
  in the subrange xx[j..j+mm-1], where xx is the stored pointer. The values in xx
  must be monotonic, either increasing or decreasing. The returned value is not
  less than 0, nor greater than xx.size()-1.
  */
int Interpolator::locate(double x)
{
    int ju, jm, jl;
    Q_ASSERT_X(!(xx.size() < 2 || mm < 2 || mm > xx.size()), "int Interpolator::locate(double x)", "locate size error");
    bool ascnd = (xx[xx.size() - 1] >= xx[0]);  // True if ascending order of table, false otherwise.
    jl = 0;                             // Initialize lower
    ju = xx.size() - 1;                         // and upper limits.
    for (; ju - jl > 1;  ) {            // If we are not yet done,
        jm = (ju + jl) >> 1;            // compute a midpoint,
        if ((x >= xx[jm]) == ascnd) {
            jl = jm;                    // and replace either the lower limit
        } else {
            ju = jm;                    // or the upper limit, as appropriate.
        }
    }                                   // Repeat until the test condition is satisfied.
    if (previousN != xx.size()) {
        dj = std::max(1, (int)pow((double)xx.size(), 0.25));
        previousN = xx.size();
    }
    cor = abs(jl - jsav) > dj ? 0 : 1;  // Decide whether to use hunt or locate next time.
    jsav = jl;
    return std::max(0, std::min(xx.size() - mm, jl - ((mm - 2) >> 1)));
}

/**
  Comments "Numerical Recipes"

  Given a value x, return a value j such that x is (insofar as possible) centered
  in the subrange xx[j..j+mm-1], where xx is the stored pointer. The values in xx
  must be monotonic, either increasing or decreasing. The returned value is not
  less than 00, nor greater than xx.size()-1.
  */
int Interpolator::hunt(double x)
{
    int jl = jsav, jm, ju, inc = 1;
    Q_ASSERT_X(!(xx.size() < 2 || mm < 2 || mm > xx.size()), "int Interpolator::hunt(double x)", "hunt size error");
    bool ascnd = (xx[xx.size() - 1] >= xx[0]);  // True if ascending order of table, false otherwise.
    if (jl < 0 || jl > xx.size() - 1) {
        jl = 0;
        ju = xx.size() - 1;
    } else {
        if ((x >= xx[jl]) == ascnd) {     // Hunt up:
            for (;;) {
                ju = jl + inc;
                if (ju >= xx.size() - 1) {      // Off end of table.
                    ju = xx.size() - 1;
                    break;
                } else if ((x < xx[ju]) == ascnd) {   // Found bracket.
                    break;
                } else if ((x == xx[ju]) && (x == xx[jl])) {
                    break;
                } else {                // Not done, so double the increment and try again.
                    jl = ju;
                    inc += inc;
                }
            }
        } else {                        // Hunt down:
            ju = jl;
            for (;;) {
                jl = jl - inc;
                if (jl <= 0) {          // Off end of table.
                    jl = 0;
                    break;
                } else if ((x >= xx[jl]) == ascnd) {  // Found bracket.
                    break;
                } else {                // Not done, so double the increment and try again.
                    ju = jl;
                    inc += inc;
                }
            }
        }
    }                                   // Hunt is done, so begin the final bisection phase:
    for (; ju - jl > 1;  ) {            // If we are not yet done,
        jm = (ju + jl) >> 1;            // compute a midpoint,
        if ((x >= xx[jm]) == ascnd) {
            jl = jm;                    // and replace either the lower limit
        } else {
            ju = jm;                    // or the upper limit, as appropriate.
        }
    }                                   // Repeat until the test condition is satisfied.
    if (previousN != xx.size()) {
        dj = std::max(1, (int)pow((double)xx.size(), 0.25));
        previousN = xx.size();
    }
    cor = abs(jl - jsav) > dj ? 0 : 1;  // Decide whether to use hunt or locate next time.
    jsav = jl;
    return std::max(0, std::min(xx.size() - mm, jl - ((mm - 2) >> 1)));
}
