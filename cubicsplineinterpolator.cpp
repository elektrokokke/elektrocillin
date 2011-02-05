#include "cubicsplineinterpolator.h"

CubicSplineInterpolator::CubicSplineInterpolator(const QVector<double> &xx, const QVector<double> &yy, double yp1, double ypn) :
    Interpolator(xx, yy, 2),
    y2(xx.size())
{
    sety2(xx, yy, yp1, ypn);
}

CubicSplineInterpolator::CubicSplineInterpolator(const QVector<double> &xx, const QVector<double> &yy) :
    Interpolator(xx, yy, 2),
    y2(xx.size())
{
    sety2NaturalSpline(xx, yy);
}

/**
  Comments from "Numerical Recipes"

  Given a value x, and using pointers to data xx and yy, and the stored vector of second
  derivatives y2, this routine returns the cubic spline interpolated value y.
  */
double CubicSplineInterpolator::interpolate(int jl, double x)
{
    int klo = jl, khi = jl + 1;
    double y, h, b, a;
    h = xx[khi] - xx[klo];
    Q_ASSERT_X(h != 0.0, "double CubicSplineInterpolator::interpolate(int jlo, double x)", "the x values of control points must be distince");
    a = (xx[khi] - x) / h;
    b = (x - xx[klo]) / h;  // Cubic spline polynomial is now evaluated.
    y = a * yy[klo] + b * yy[khi] + ((a * a * a - a) * y2[klo] + (b * b * b - b) * y2[khi]) * (h * h) / 6.0;
    return y;
}

/**
  Comments from "Numerical Recipes"

  This routine stores an array y2[0..n-1] with second derivatices of the interpolating function
  at the tabulated points pointed to by xx, using function values pointed to by yy. If yp1
  and/or ypn are equal to 1x10^99 or larger, the routine is signaled to set the corresponding
  boundary condition for a natural spline, with zero second derivatives on that boundary; otherwise,
  they are the values of the first derivatives at the endpoints.

  Note: in contrast to the original implementation, we provide two methods here, one for the
  given first derivative yp1 and ypn (sety2()), and the other one for a natural spline
  (sety2NaturalSpline()).)
  */
void CubicSplineInterpolator::sety2(const QVector<double> &xx, const QVector<double> &yy, double yp1, double ypn)
{
    double p, qn, sig, un;
    QVector<double> u(n - 1);
                                        // The lower boundary condition is set either to be "natural" (see sety2NaturalSpline())
    y2[0] = -0.5;                       // or else to have a specified first derivative.
    u[0] = (3.0 / (xx[1] - xx[0])) * ((yy[1] - yy[0]) / (xx[1] - xx[0]) - yp1);
    for (int i = 1; i < n - 1; i++) {   // This is the decomposition loop of the tridiagonal algorithm. y2 and u are used for temporary storage of the decomposed factors.
        sig = (xx[i] - xx[i - 1]) / (xx[i + 1] - xx[i - 1]);
        p = sig * y2[i - 1] + 2.0;
        y2[i] = (sig - 1.0) / p;
        u[i] = (yy[i + 1] - yy[i]) / (xx[i+ 1] - xx[i]) - (yy[i] - yy[i - 1]) / (xx[i] - xx[i - 1]);
        u[i] = (6.0 * u[i] / (xx[i + 1] - xx[i - 1]) - sig * u[i - 1]) / p;
    }
                                        // The upper boundary condition is set either to be "natural" (see sety2NaturalSpline())
    qn = 0.5;                           // or else to have a specified first derivative.
    un = (3.0 / (xx[n - 1] - xx[n - 2])) * (ypn - (yy[n - 1] - yy[n - 2]) / (xx[n - 1] - xx[n - 2]));
    y2[n - 1] = (un - qn * u[n - 2]) / (qn * y2[n - 2] + 1.0);
    for (int k = n - 2; k >= 0; k--) {  // This is the backsubstitution loop of the trtidiagonal algorithm.
        y2[k] = y2[k] * y2[k + 1] + u[k];
    }
}

void CubicSplineInterpolator::sety2NaturalSpline(const QVector<double> &xx, const QVector<double> &yy)
{
    double p, qn, sig, un;
    QVector<double> u(n - 1);
    y2[0] = u[0] = 0.0;                 // The lower boundary condition is set either to be "natural"
                                        // or else to have a specified first derivative (see sety2()).
    for (int i = 1; i < n - 1; i++) {   // This is the decomposition loop of the tridiagonal algorithm. y2 and u are used for temporary storage of the decomposed factors.
        sig = (xx[i] - xx[i - 1]) / (xx[i + 1] - xx[i - 1]);
        p = sig * y2[i - 1] + 2.0;
        y2[i] = (sig - 1.0) / p;
        u[i] = (yy[i + 1] - yy[i]) / (xx[i+ 1] - xx[i]) - (yy[i] - yy[i - 1]) / (xx[i] - xx[i - 1]);
        u[i] = (6.0 * u[i] / (xx[i + 1] - xx[i - 1]) - sig * u[i - 1]) / p;
    }
    qn = un = 0.0;                      // The upper boundary condition is set either to be "natural"
                                        // or else to have a specified first derivative (see sety2()).
    y2[n - 1] = (un - qn * u[n - 2]) / (qn * y2[n - 2] + 1.0);
    for (int k = n - 2; k >= 0; k--) {  // This is the backsubstitution loop of the trtidiagonal algorithm.
        y2[k] = y2[k] * y2[k + 1] + u[k];
    }
}
