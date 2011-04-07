#ifndef INTERPOLATOR_H
#define INTERPOLATOR_H

/*
    Copyright 2011 Arne Jacobs <jarne@jarne.de>

    This file is part of elektrocillin.

    Elektrocillin is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QVector>
#include <QMap>
#include <QPair>
#include <QPointF>
#include "eventprocessor.h"

class AbstractInterpolator
{
public:
    virtual double evaluate(double x, int *index = 0) = 0;
    virtual int getNrOfControlPoints() = 0;
    virtual QPointF getControlPoint(int index) = 0;
    virtual void changeControlPoint(int index, double x, double y) = 0;
    virtual void addControlPoint(double x, double y) = 0;
    virtual void deleteControlPoint(int index) = 0;
    virtual QString getControlPointName(int index) const = 0;
};

class Interpolator : public AbstractInterpolator
{
public:
    class InterpolatorEvent : public RingBufferEvent
    {
    public:
        virtual ~InterpolatorEvent() {}
    };
    class ChangeControlPointEvent : public InterpolatorEvent
    {
    public:
        ChangeControlPointEvent(int index_, double x_, double y_) :
            index(index_), x(x_), y(y_)
        {}
        int index;
        double x, y;
    };
    class AddControlPointEvent : public InterpolatorEvent
    {
    public:
        AddControlPointEvent(double x_, double y_) :
            x(x_), y(y_)
        {}
        double x, y;
    };
    class DeleteControlPointEvent : public InterpolatorEvent
    {
    public:
        DeleteControlPointEvent(int index_) :
            index(index_)
        {}
        int index;
    };

    virtual ~Interpolator();

    // AbstractInterpolator interface:
    /**
      @param index pointer to a variable where the current
        index should be written to, if non-zero
      */
    double evaluate(double x, int *index = 0);
    virtual int getNrOfControlPoints();
    virtual QPointF getControlPoint(int index);
    virtual void changeControlPoint(int index, double x, double y);
    virtual void addControlPoint(double x, double y);
    virtual void deleteControlPoint(int index);
    virtual QString getControlPointName(int index) const;

    /**
      Reimplement this method in your interpolator class.
      */
    virtual double interpolate(int jlo, double x) = 0;

    // additional methods:
    void setControlPointName(int controlPointIndex, const QString &name);
    void setMonotonicity(bool isStrictlyMonotonic);
    void setStartPointConstraints(bool xIsStatic, bool yIsStatic);
    void setEndPointConstraints(bool xIsStatic, bool yIsStatic);
    void setYRange(double yMin, double yMax);
    /**
      Resets the interpolator in a way that traversing it from
      start to end becomes more efficient.
      */
    void reset();
    const QVector<double> & getX() const;
    const QVector<double> & getY() const;
    int getM() const;

    virtual void save(QDataStream &stream) const;
    virtual void load(QDataStream &stream);
    virtual void changeControlPoints(const QVector<double> &xx, const QVector<double> &yy);
    virtual void processInterpolatorEvent(const InterpolatorEvent *event);
protected:
    Interpolator(const QVector<double> &xx, const QVector<double> &yy, int m);

    /**
      Reimplement this method if you have to recompute something
      internally when one or more control points change.
      */
    virtual void controlPointsChanged();

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
