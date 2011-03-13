#include "linearintegralinterpolator.h"

LinearIntegralInterpolator::LinearIntegralInterpolator(const LinearInterpolator &linear) :
    Interpolator(linear.getX(), linear.getY(), linear.getM()),
    a(linear.getX().size() - 1),
    b(linear.getX().size() - 1),
    c(linear.getX().size() - 1)
{
    // compute coefficients of the quadratic functions which make up the integral:
    double previousIntegralValue = 0;
    for (int i = 0; i < a.size(); i++) {
        if (getX()[i] == getX()[i + 1]) {
            // discontinuity in the function, insert "empty" quadratic:
            a[i] = b[i] = 0;
            c[i] = previousIntegralValue;
        } else {
            // f'(x) = 2 * a * x + b
            a[i] = 0.5 * (linear.getY()[i] - linear.getY()[i + 1]) / (getX()[i] - getX()[i + 1]);
            b[i] = linear.getY()[i] - 2.0 * a[i] * linear.getX()[i];
            // f(x) = a * x^2 + b * x + c
            c[i] = previousIntegralValue - a[i] * getX()[i] * getX()[i] - b[i] * getX()[i];
            previousIntegralValue = a[i] * getX()[i + 1] * getX()[i + 1] + b[i] * getX()[i + 1] + c[i];
        }
    }
}

void LinearIntegralInterpolator::save(QDataStream &stream) const
{
    Interpolator::save(stream);
    stream << a << b << c;
}

void LinearIntegralInterpolator::load(QDataStream &stream)
{
    Interpolator::load(stream);
    stream >> a >> b >> c;
}

double LinearIntegralInterpolator::interpolate(int jlo, double x)
{
    return a[jlo] * x * x + b[jlo] * x + c[jlo];
}
