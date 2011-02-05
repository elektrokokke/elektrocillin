#ifndef CUBICSPLINEINTERPOLATOR_H
#define CUBICSPLINEINTERPOLATOR_H

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

protected:
    double interpolate(int jlo, double x);

private:
    QVector<double> y2;

    void sety2(const QVector<double> &xx, const QVector<double> &yy, double yp1, double ypn);
    void sety2NaturalSpline(const QVector<double> &xx, const QVector<double> &yy);
};

#endif // CUBICSPLINEINTERPOLATOR_H
