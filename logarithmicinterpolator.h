#ifndef LOGARITHMICINTERPOLATOR_H
#define LOGARITHMICINTERPOLATOR_H

#include "interpolator.h"

class LogarithmicInterpolator : public Interpolator
{
public:
    LogarithmicInterpolator(double base);
    LogarithmicInterpolator(const QVector<double> &xx, const QVector<double> &yy, double base);

    void save(QDataStream &stream) const;
    void load(QDataStream &stream);

    const QVector<double> & getX() const;
    const QVector<double> & getY() const;
    QVector<double> & getX();
    QVector<double> & getY();

    double interpolate(int jlo, double x);

    virtual void processEvent(const Interpolator::ChangeControlPointEvent *event);
    virtual void processEvent(const Interpolator::ChangeAllControlPointsEvent *event);
private:
    QVector<double> yy;
    double base;
};

#endif // LOGARITHMICINTERPOLATOR_H
