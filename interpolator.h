#ifndef INTERPOLATOR_H
#define INTERPOLATOR_H

#include "jackringbuffer.h"
#include <QVector>

class Interpolator
{
public:
    class ChangeControlPointEvent : public RingBufferEvent
    {
    public:
        int index;
        double x, y;
    };
    class ChangeAllControlPointsEvent : public RingBufferEvent
    {
    public:
        QVector<double> xx, yy;
    };

    /**
      @param index pointer to a variable where the current
        index should be written to, if non-zero
      */
    double evaluate(double x, int *index = 0);

    const QVector<double> & getX() const;
    int getM() const;

    virtual double interpolate(int jlo, double x) = 0;

    virtual void processEvent(const ChangeControlPointEvent *event) = 0;
    virtual void processEvent(const ChangeAllControlPointsEvent *event) = 0;
protected:
    Interpolator(const QVector<double> &xx, int m);

    int locate(double x);
    int hunt(double x);

    QVector<double> xx;
private:
    int mm, jsav, cor, dj, previousN;
};

#endif // INTERPOLATOR_H
