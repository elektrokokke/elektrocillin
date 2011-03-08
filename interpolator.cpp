#include "interpolator.h"
#include <cmath>

Interpolator::~Interpolator()
{
}

void Interpolator::setControlPointName(int controlPointIndex, const QString &name)
{
    Q_ASSERT((controlPointIndex >= 0) && (controlPointIndex < xx.size()));
    names[controlPointIndex] = name;
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

void Interpolator::changeControlPoints(const QVector<double> &xx, const QVector<double> &yy)
{
    // note: no check is done here wether the given points adhere to all constraints
    // (e.g., the monotonicity constraints)
    this->xx = xx;
    this->yy = yy;
}

void Interpolator::changeControlPoint(const ChangeControlPointEvent *event)
{
    Q_ASSERT((event->index >= 0) && (event->index < xx.size()));
    double x = event->x;
    double y = event->y;
    if (event->index == 0) {
        // fix the start point if desired:
        if (xIsStatic.first) {
            x = xx.first();
        }
        if (yIsStatic.first) {
            y = yy.first();
        }
    } else {
        // make sure that all x values increase monotonically:
        if (isStrictlyMonotonic && (x <= xx[event->index - 1])) {
            return;
        } else if (x < xx[event->index - 1]) {
            x = xx[event->index - 1];
        }
    }
    if (event->index == xx.size() - 1) {
        // fix the end points if desired:
        if (xIsStatic.second) {
            x = xx.last();
        }
        if (yIsStatic.second) {
            y = yy.last();
        }
    } else {
        // make sure that all x values increase monotonically:
        if (isStrictlyMonotonic && (x >= xx[event->index + 1])) {
            return;
        } else if (x > xx[event->index + 1]) {
            x = xx[event->index + 1];
        }
    }
    // enforce the y range constraint:
    if (y < yMin) {
        y = yMin;
    }
    if (y > yMax) {
        y = yMax;
    }
    xx[event->index] = x;
    yy[event->index] = y;
}

void Interpolator::addControlPoints(const AddControlPointsEvent *event)
{
    Q_ASSERT(event->addAtStart || event->addAtEnd);
    int origin = xx.size() / 2;
    if (!event->addAtEnd) {
        origin = xx.size() - 1;
    } else if (!event->addAtStart) {
        origin = 0;
    }
    if (event->addAtEnd) {
        int pointsAfterOrigin = xx.size() - origin;
        double x = xx.last();
        double y = yy.last();
        // scale the points after the origin:
        double scaleFactor = (double)pointsAfterOrigin / (double)(pointsAfterOrigin + 1);
        for (int i = origin + 1; i < xx.size(); i++) {
            if (event->scaleX) {
                xx[i] = (xx[i] - xx[origin]) * scaleFactor + xx[origin];
            }
            if (event->scaleY) {
                yy[i] = (yy[i] - yy[origin]) * scaleFactor + yy[origin];
            }
        }
        // add one at the end:
        xx.append(x);
        yy.append(y);
    }
    if (event->addAtStart) {
        int pointsBeforeOrigin = origin;
        double x = xx.first();
        double y = yy.first();
        // scale the points before the origin:
        double scaleFactor = (double)pointsBeforeOrigin / (double)(pointsBeforeOrigin + 1);
        for (int i = 0; i < origin; i++) {
            if (event->scaleX) {
                xx[i] = (xx[i] - xx[origin]) * scaleFactor + xx[origin];
            }
            if (event->scaleY) {
                yy[i] = (yy[i] - yy[origin]) * scaleFactor + yy[origin];
            }
        }
        // add one at the start:
        xx.insert(0, x);
        yy.insert(0, y);
    }
}

void Interpolator::deleteControlPoints(const DeleteControlPointsEvent *event)
{
    Q_ASSERT(event->deleteAtStart || event->deleteAtEnd);
    int origin = xx.size() / 2;
    if (!event->deleteAtEnd) {
        origin = xx.size() - 1;
    } else if (!event->deleteAtStart) {
        origin = 0;
    }
    if (event->deleteAtEnd) {
        int pointsAfterOrigin = xx.size() - origin;
        // only remove a point if there are enough:
        if (pointsAfterOrigin > 1) {
            double x = xx.last();
            double y = yy.last();
            // remove one point at the end:
            xx.remove(xx.size() - 1);
            yy.remove(yy.size() - 1);
            // scale the remaining ones after the origin:
            double scaleFactor = (double)pointsAfterOrigin / (double)(pointsAfterOrigin - 1);
            for (int i = origin + 1; i < xx.size(); i++) {
                if (event->scaleX) {
                    xx[i] = (xx[i] - xx[origin]) * scaleFactor + xx[origin];
                }
                if (event->scaleY) {
                    yy[i] = (yy[i] - yy[origin]) * scaleFactor + yy[origin];
                }
            }
            // enforce the end point constraints:
            if (xIsStatic.second) {
                xx.last() = x;
            }
            if (yIsStatic.second) {
                yy.last() = y;
            }
        }
    }
    if (event->deleteAtStart) {
        int pointsBeforeOrigin = origin;
        // only remove a point if there are enough:
        if (pointsBeforeOrigin > 1) {
            double x = xx.first();
            double y = yy.first();
            // remove one point at the start:
            xx.remove(0);
            yy.remove(0);
            origin--;
            // scale the remaining ones before the origin:
            double scaleFactor = (double)pointsBeforeOrigin / (double)(pointsBeforeOrigin + 1);
            for (int i = 0; i < origin; i++) {
                if (event->scaleX) {
                    xx[i] = (xx[i] - xx[origin]) * scaleFactor + xx[origin];
                }
                if (event->scaleY) {
                    yy[i] = (yy[i] - yy[origin]) * scaleFactor + yy[origin];
                }
            }
            // enforce the start point constraints:
            if (xIsStatic.first) {
                xx.first() = x;
            }
            if (yIsStatic.first) {
                yy.first() = y;
            }
        }
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

void Interpolator::changeControlPoint(int index, double x, double y)
{
    ChangeControlPointEvent event(index, x, y);
    changeControlPoint(&event);
}

void Interpolator::addControlPoints(bool scaleX, bool scaleY, bool addAtStart, bool addAtEnd)
{
    AddControlPointsEvent event(scaleX, scaleY, addAtStart, addAtEnd);
    addControlPoints(&event);
}

void Interpolator::deleteControlPoints(bool scaleX, bool scaleY, bool deleteAtStart, bool deleteAtEnd)
{
    DeleteControlPointsEvent event(scaleX, scaleY, deleteAtStart, deleteAtEnd);
    deleteControlPoints(&event);
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
