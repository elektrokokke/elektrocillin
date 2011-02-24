#ifndef LINEARINTERPOLATOR_H
#define LINEARINTERPOLATOR_H

#include "interpolator.h"
#include <QVector>

class LinearInterpolator : public Interpolator
{
public:
    LinearInterpolator();
    LinearInterpolator(const QVector<double> &xx, const QVector<double> &yy);

    const QVector<double> & getX() const;
    const QVector<double> & getY() const;
    QVector<double> & getX();
    QVector<double> & getY();

    double interpolate(int jlo, double x);

    virtual void processEvent(const Interpolator::ChangeControlPointEvent *event);
    virtual void processEvent(const Interpolator::ChangeAllControlPointsEvent *event);
private:
    QVector<double> yy;
};

#endif // LINEARINTERPOLATOR_H
