#ifndef INTERPOLATOR_H
#define INTERPOLATOR_H

#include <QVector>

class Interpolator
{
public:
    double evaluate(double x);

protected:
    Interpolator(const QVector<double> &xx, const QVector<double> &yy, int m);

    int locate(double x);
    int hunt(double x);

    virtual double interpolate(int jlo, double x) = 0;

    const QVector<double> xx, yy;
    int n;
private:
    int mm, jsav, cor, dj;
};

#endif // INTERPOLATOR_H
