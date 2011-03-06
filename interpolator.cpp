#include "interpolator.h"
#include <cmath>

Interpolator::~Interpolator()
{
}

double Interpolator::evaluate(double x, int *index)
{
    int jlo = cor ? hunt(x) : locate(x);
    if (index) {
        *index = jlo;
    }
    return interpolate(jlo, x);
}

const QVector<double> & Interpolator::getX() const
{
    return xx;
}

int Interpolator::getM() const {
    return mm;
}

Interpolator::Interpolator(const QVector<double> &xx_, int m_) :
    xx(xx_),
    mm(m_),
    jsav(0),
    cor(0)
{
    dj = std::max(1, (int)pow((double)xx.size(), 0.25));
    previousN = xx.size();
}

/**
  Comments from "Numerical Recipes"

  Given a valu x, return a value j such that x is (insofar as possible) centered
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
