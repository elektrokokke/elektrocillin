#ifndef LINEARINTEGRALINTERPOLATOR_H
#define LINEARINTEGRALINTERPOLATOR_H

#include "linearinterpolator.h"

/**
  Represents the indefinite integral of a piecewise linear function,
  whose parameters are given via a LinearInterpolator object.
  */

class LinearIntegralInterpolator : public Interpolator
{
public:
    LinearIntegralInterpolator(const LinearInterpolator &linearInterpolator);

    virtual void save(QDataStream &stream) const;
    virtual void load(QDataStream &stream);

    virtual double interpolate(int jlo, double x);

private:
    QVector<double> a, b, c;
};

#endif // LINEARINTEGRALINTERPOLATOR_H
