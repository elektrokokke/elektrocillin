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

    const QVector<double> & getX() const;
    const QVector<double> & getY() const;
    QVector<double> & getX();
    QVector<double> & getY();

    double interpolate(int jlo, double x);

    virtual void processEvent(const Interpolator::ChangeControlPointEvent *event);
    virtual void processEvent(const Interpolator::ChangeAllControlPointsEvent *event);
};

#endif // LINEARINTERPOLATOR_H
