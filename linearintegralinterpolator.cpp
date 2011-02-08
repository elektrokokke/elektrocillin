#include "linearintegralinterpolator.h"

LinearIntegralInterpolator::LinearIntegralInterpolator(const LinearInterpolator &linear) :
    Interpolator(linear.getX(), linear.getM()),
    a(linear.getX().size() - 1),
    b(linear.getX().size() - 1),
    c(linear.getX().size() - 1)
{
    // compute coefficients of the quadratic functions which make up the integral:
    double previousIntegralValue = 0;
    for (int i = 0; i < a.size(); i++) {
        if (linear.getX()[i] == linear.getX()[i + 1]) {
            // discontinuity in the function, insert "empty" quadratic:
            a[i] = b[i] = 0;
            c[i] = previousIntegralValue;
        } else {
            // f'(x) = 2 * a * x + b
            a[i] = 0.5 * (linear.getY()[i] - linear.getY()[i + 1]) / (linear.getX()[i] - linear.getX()[i + 1]);
            b[i] = linear.getY()[i] - 2.0 * a[i] * linear.getX()[i];
            // f(x) = a * x^2 + b * x + c
            c[i] = previousIntegralValue - a[i] * linear.getX()[i] * linear.getX()[i] - b[i] * linear.getX()[i];
            previousIntegralValue = a[i] * linear.getX()[i + 1] * linear.getX()[i + 1] + b[i] * linear.getX()[i + 1] + c[i];
        }
    }
}

double LinearIntegralInterpolator::interpolate(int jlo, double x)
{
    return a[jlo] * x * x + b[jlo] * x + c[jlo];
}
