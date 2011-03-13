#ifndef LOGARITHMICINTERPOLATOR_H
#define LOGARITHMICINTERPOLATOR_H

#include "interpolator.h"

class LogarithmicInterpolator : public Interpolator
{
public:
    LogarithmicInterpolator(double base);
    LogarithmicInterpolator(const QVector<double> &xx, const QVector<double> &yy, double base);

    virtual void save(QDataStream &stream) const;
    virtual void load(QDataStream &stream);

    virtual double interpolate(int jlo, double x);
private:
    double base;
};

#endif // LOGARITHMICINTERPOLATOR_H
