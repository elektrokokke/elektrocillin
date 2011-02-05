#ifndef LINEARINTERPOLATOR_H
#define LINEARINTERPOLATOR_H

#include "interpolator.h"
#include <QVector>

class LinearInterpolator : public Interpolator
{
public:
    LinearInterpolator(const QVector<double> &xx, const QVector<double> &yy);

    QVector<double> & getX();
    QVector<double> & getY();

protected:
    double interpolate(int jlo, double x);
};

#endif // LINEARINTERPOLATOR_H
