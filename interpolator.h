#ifndef INTERPOLATOR_H
#define INTERPOLATOR_H

#include "jackringbuffer.h"
#include <QVector>
#include <QMap>
#include <QPair>

class Interpolator
{
public:
    class ChangeControlPointEvent : public RingBufferEvent
    {
    public:
        ChangeControlPointEvent(int index_, double x_, double y_) :
            index(index_), x(x_), y(y_)
        {}
        int index;
        double x, y;
    };
    class AddControlPointsEvent : public RingBufferEvent
    {
    public:
        AddControlPointsEvent(bool scaleX_, bool scaleY_, bool addAtStart_, bool addAtEnd_) :
            scaleX(scaleX_), scaleY(scaleY_), addAtStart(addAtStart_), addAtEnd(addAtEnd_)
        {}
        bool scaleX, scaleY, addAtStart, addAtEnd;
    };
    class DeleteControlPointsEvent : public RingBufferEvent
    {
    public:
        DeleteControlPointsEvent(bool scaleX_, bool scaleY_, bool deleteAtStart_, bool deleteAtEnd_) :
            scaleX(scaleX_), scaleY(scaleY_), deleteAtStart(deleteAtStart_), deleteAtEnd(deleteAtEnd_)
        {}
        bool scaleX, scaleY, deleteAtStart, deleteAtEnd;
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

    virtual void changeControlPoints(const QVector<double> &xx, const QVector<double> &yy);
    virtual void changeControlPoint(const ChangeControlPointEvent *event);
    virtual void addControlPoints(const AddControlPointsEvent *event);
    virtual void deleteControlPoints(const DeleteControlPointsEvent *event);
    // overloaded convenience methods for the above:
    void changeControlPoint(int index, double x, double y);
    void addControlPoints(bool scaleX, bool scaleY, bool addAtStart, bool addAtEnd);
    void deleteControlPoints(bool scaleX, bool scaleY, bool deleteAtStart, bool deleteAtEnd);

    void setMonotonicity(bool isStrictlyMonotonic);
    void setStartPointConstraints(bool xIsStatic, bool yIsStatic);
    void setEndPointConstraints(bool xIsStatic, bool yIsStatic);
    void setYRange(double yMin, double yMax);

//    virtual void processEvent(const ChangeAllControlPointsEvent *event) = 0;
protected:
    Interpolator(const QVector<double> &xx, const QVector<double> &yy, int m);

    int locate(double x);
    int hunt(double x);

    QVector<double> xx, yy;
private:
    int mm, jsav, cor, dj, previousN;
    QMap<int, QString> names;
    bool isStrictlyMonotonic;
    QPair<bool, bool> xIsStatic, yIsStatic;
    double yMin, yMax;
};

#endif // INTERPOLATOR_H
