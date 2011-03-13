#ifndef INTERPOLATOR_H
#define INTERPOLATOR_H

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
