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
    /**
      Constructor with given second derivatives.
      */
    CubicSplineInterpolator(const QVector<double> &xx, const QVector<double> &yy, const QVector<double> &y2);

    void save(QDataStream &stream);
    void load(QDataStream &stream);

    const QVector<double> & getY() const;
    const QVector<double> & getY2() const;

    double interpolate(int jlo, double x);

    virtual void processEvent(const ChangeControlPointEvent *event);
    virtual void processEvent(const ChangeAllControlPointsEvent *event);
private:
    QVector<double> yy, y2;

    void sety2(const QVector<double> &xx, const QVector<double> &yy, double yp1, double ypn);
    void sety2NaturalSpline(const QVector<double> &xx, const QVector<double> &yy);
};

#endif // CUBICSPLINEINTERPOLATOR_H
