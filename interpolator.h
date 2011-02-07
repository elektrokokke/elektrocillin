#ifndef INTERPOLATOR_H
#define INTERPOLATOR_H

#include <QVector>

class Interpolator
{
public:
    /**
      @param index pointer to a variable where the current
        index should be written to, if non-zero
      */
    double evaluate(double x, int *index = 0);

    const QVector<double> & getX() const;
    int getM() const;

    virtual double interpolate(int jlo, double x) = 0;
protected:
    Interpolator(const QVector<double> &xx, int m);

    int locate(double x);
    int hunt(double x);

    QVector<double> xx;
    int n;
private:
    int mm, jsav, cor, dj;
};

#endif // INTERPOLATOR_H
