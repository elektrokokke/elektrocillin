#ifndef LINEARINTERPOLATOR_H
#define LINEARINTERPOLATOR_H

#include "interpolator.h"
#include <QVector>
#include <QDataStream>

class LinearInterpolator : public Interpolator
{
public:
    LinearInterpolator();
    LinearInterpolator(const QVector<double> &xx, const QVector<double> &yy);

    void save(QDataStream &stream) const;
    void load(QDataStream &stream);

    double interpolate(int jlo, double x);
};

#endif // LINEARINTERPOLATOR_H
