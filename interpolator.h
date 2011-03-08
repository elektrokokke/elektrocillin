#ifndef INTERPOLATOR_H
#define INTERPOLATOR_H

#include "jackringbuffer.h"
#include <QVector>
#include <QMap>

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
        ChangeAllControlPointsEvent() {}
        ChangeAllControlPointsEvent(const Interpolator &interpolator) :
            xx(interpolator.getX()),
            yy(interpolator.getY())
        {}
        QVector<double> xx, yy;
    };

    virtual ~Interpolator();

    void setControlPointName(int controlPointIndex, const QString &name);
    QString getControlPointName(int controlPointIndex) const;

    /**
      @param index pointer to a variable where the current
        index should be written to, if non-zero
      */
    double evaluate(double x, int *index = 0);

    /**
      Resets the interpolator in a way that traversing it from
      start to end becomes more efficient.
      */
    void reset();

    const QVector<double> & getX() const;
    const QVector<double> & getY() const;
    int getM() const;

    virtual double interpolate(int jlo, double x) = 0;

    virtual void processEvent(const ChangeControlPointEvent *event) = 0;
    virtual void processEvent(const ChangeAllControlPointsEvent *event) = 0;
protected:
    Interpolator(const QVector<double> &xx, const QVector<double> &yy, int m);

    int locate(double x);
    int hunt(double x);

    QVector<double> xx, yy;
private:
    int mm, jsav, cor, dj, previousN;
    QMap<int, QString> names;
};

#endif // INTERPOLATOR_H
