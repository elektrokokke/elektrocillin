#include "linearinterpolator.h"

LinearInterpolator::LinearInterpolator() :
    Interpolator(QVector<double>(), 2)
{
}

LinearInterpolator::LinearInterpolator(const QVector<double> &xx, const QVector<double> &yy_) :
    Interpolator(xx, 2),
    yy(yy_)
{
}

const QVector<double> & LinearInterpolator::getX() const
{
    return Interpolator::getX();
}

const QVector<double> & LinearInterpolator::getY() const
{
    return yy;
}

QVector<double> & LinearInterpolator::getX()
{
    return xx;
}

QVector<double> & LinearInterpolator::getY()
{
    return yy;
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
    if (j + 1 >= xx.size()) {
        j = xx.size() - 2;
    }
    if (xx[j] == xx[j + 1]) {   // Table is defective, but we can recover.
        return yy[j];
    } else {
        return yy[j] + ((x - xx[j]) / (xx[j + 1] - xx[j])) * (yy[j + 1] - yy[j]);
    }
}
