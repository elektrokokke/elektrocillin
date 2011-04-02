#ifndef INTERPOLATOR_H
#define INTERPOLATOR_H

/*
    Copyright 2011 Arne Jacobs

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

class Interpolator
{
public:
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

    virtual void save(QDataStream &stream) const;
    virtual void load(QDataStream &stream);

    virtual double interpolate(int jlo, double x) = 0;

    virtual void changeControlPoints(const QVector<double> &xx, const QVector<double> &yy);
    virtual void changeControlPoint(int index, double x, double y);
    virtual void addControlPoints(bool scaleX, bool scaleY, bool addAtStart, bool addAtEnd);
    virtual void deleteControlPoints(bool scaleX, bool scaleY, bool deleteAtStart, bool deleteAtEnd);

    void setMonotonicity(bool isStrictlyMonotonic);
    void setStartPointConstraints(bool xIsStatic, bool yIsStatic);
    void setEndPointConstraints(bool xIsStatic, bool yIsStatic);
    void setYRange(double yMin, double yMax);
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
