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

